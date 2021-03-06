
// Gb_Snd_Emu 0.1.4. http://www.slack.net/~ant/

#include "Sound_Queue.h"

#include <assert.h>
#include <string.h>

#include "../../../non_core/logger.h"

#ifdef DREAMCAST
#include <cstdlib>
#endif

/* Copyright (C) 2005 by Shay Green. Permission is hereby granted, free of
charge, to any person obtaining a copy of this software module and associated
documentation files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the
following conditions: The above copyright notice and this permission notice
shall be included in all copies or substantial portions of the Software. THE
SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */


// Return current SDL_GetError() string, or str if SDL didn't have a string
static const char* sdl_error( const char* str )
{
	const char* sdl_str = SDL_GetError();
	if ( sdl_str && *sdl_str )
		str = sdl_str;
	return str;
}

Sound_Queue::Sound_Queue()
{
	bufs = NULL;
	free_sem = NULL;
	sound_open = false;
}

Sound_Queue::~Sound_Queue()
{
	stop();
}

const char* Sound_Queue::start( long sample_rate, int chan_count )
{
	assert( !bufs ); // can only be initialized once
	
	write_buf = 0;
	write_pos = 0;
	read_buf = 0;
	
#ifndef DREAMCAST
	bufs = new sample_t [(long) buf_size * buf_count];
#else
    bufs = (sample_t *)std::malloc((long) buf_size * buf_count); 
#endif	
    if ( !bufs ) {
	    log_message(LOG_ERROR, "Run out of memory starting sound queue\n"); 
    	return "Out of memory";
    }
	currently_playing_ = bufs;

#ifndef EMSCRIPTEN	
	free_sem = SDL_CreateSemaphore( buf_count - 1 );
	if ( !free_sem ) {
	    log_message(LOG_ERROR, "Couldn't create semaphore starting sound queue. %s\n", SDL_GetError()); 
    	return sdl_error( "Couldn't create semaphore" );
	}
#endif

	SDL_AudioSpec as;
	as.freq = sample_rate;
	as.format = AUDIO_S16SYS;
	as.channels = chan_count;
	as.silence = 0;
	as.samples = buf_size / chan_count;
	as.size = 0;
#ifndef EMSCRIPTEN
    as.callback = fill_buffer_;
#else // If you try to output Audio on a non-main thread in emscripten, you're going to
     //  have a bad time
    as.callback = NULL; //fill_buffer_;
#endif	
    as.userdata = this;
    
    SDL_AudioSpec as2;

#ifndef EMSCRIPTEN
    if (!(this->device = SDL_OpenAudioDevice(NULL, 0, &as, &as2, SDL_AUDIO_ALLOW_ANY_CHANGE))) 
#else
//    if (!(this->device = SDL_OpenAudioDevice(NULL, 0, &as, &as2, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE))) 
    if (!(this->device = SDL_OpenAudioDevice(NULL, 0, &as, &as2, 0))) 
#endif
    {
        log_message(LOG_ERROR, "Couldn't open SDL audio\n"); 
        return sdl_error( "Couldn't open SDL audio" );
    }
    
    SDL_PauseAudioDevice(this->device, 0);
	sound_open = true;
	
	return NULL;
}

void Sound_Queue::stop()
{
	if ( sound_open )
	{
		sound_open = false;
		SDL_PauseAudioDevice(this->device, 1);
		SDL_CloseAudioDevice(this->device);
	}
	
	if ( free_sem )
	{
		SDL_DestroySemaphore( free_sem );
		free_sem = NULL;
	}
#ifndef DREAMCAST
	delete [] bufs;
#else
    std::free(bufs);
#endif
	bufs = NULL;
}

int Sound_Queue::sample_count() const
{
#ifndef EMSCRIPTEN
	int buf_free = SDL_SemValue(free_sem) * buf_size + (buf_size - write_pos);

#else
	int buf_free = (buf_count - 1) * buf_size + (buf_size - write_pos);
#endif   
    return buf_size * buf_count - buf_free;
}

inline Sound_Queue::sample_t* Sound_Queue::buf( int index )
{
	assert( (unsigned) index < buf_count );
	return bufs + (long) index * buf_size;
}

void Sound_Queue::write( const sample_t* in, int count )
{
    while ( count )
	{
		int n = buf_size - write_pos;
		if ( n > count )
			n = count;
		
		memcpy( buf( write_buf ) + write_pos, in, n * sizeof (sample_t) );
		in += n;
		write_pos += n;
		count -= n;
		
		if ( write_pos >= buf_size )
		{
			write_pos = 0;
			write_buf = (write_buf + 1) % buf_count;
        #ifndef EMSCRIPTEN
           SDL_SemWait( free_sem );
        #else // Queue Audio in our main thread
            SDL_QueueAudio(this->device, buf(write_buf), buf_size * sizeof (sample_t));
		#endif
        }
	}
}

void Sound_Queue::fill_buffer( Uint8* out, int count )
{
	if ( SDL_SemValue( free_sem ) < buf_count - 1 )
	{
		currently_playing_ = buf( read_buf );
		memcpy( out, buf( read_buf ), count );
		read_buf = (read_buf + 1) % buf_count;
		SDL_SemPost( free_sem );
	}
	else
	{
		memset( out, 0, count );
	}
}

void Sound_Queue::fill_buffer_( void* user_data, Uint8* out, int count )
{
	((Sound_Queue*) user_data)->fill_buffer( out, count );
}

