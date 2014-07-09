/*
 * ALSA Capture Test
 *
 * Copyright (c) 2014 Alexei A. Smekalkine
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

#define LATENCY  1000  /* 1ms */

int main (int argc, char *argv[])
{
	int rc;
	snd_pcm_t *pcm;
	snd_pcm_uframes_t buffer_size, period_size;
	void *buffer;
	unsigned long loops;
	size_t count;

	/* We do not want garbage on the terminal */
	if (isatty (1)) {
		fprintf (stderr, "usage:\n\talsa-capture > output-file\n");
		return 1;
	}

	/* Open and initialize PCM device */
	rc = snd_pcm_open (&pcm, "default", SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0)
		goto no_device;

	rc = snd_pcm_set_params (pcm, SND_PCM_FORMAT_S16_LE,
				 SND_PCM_ACCESS_RW_INTERLEAVED, 2, 44100,
				 1, LATENCY);
	if (rc < 0)
		goto no_set_params;

	rc = snd_pcm_get_params (pcm, &buffer_size, &period_size);
	if (rc < 0)
		goto no_get_params;

	/* Allocate software buffer */
	count = snd_pcm_frames_to_bytes (pcm, period_size);
	buffer = malloc (count);
	if (buffer == NULL) {
		perror ("buffer allocation");
		return 1;
	}

	/* Main loop, 5 seconds run */
	for (loops = 5000000 / LATENCY; loops > 0; --loops) {
		rc = snd_pcm_readi (pcm, buffer, period_size);
		if (rc < 0) {
			if (snd_pcm_recover (pcm, rc, 1) < 0)
				fprintf (stderr, "pcm read error: %s\n",
					 snd_strerror(rc));
			/* Try to ignore errors */
		}
		else {
			count = snd_pcm_frames_to_bytes (pcm, rc);
			write (1, buffer, count);
		}
	}

	snd_pcm_drain (pcm);
	snd_pcm_close (pcm);
	free (buffer);
	return 0;

no_get_params:
no_set_params:
	snd_pcm_close (pcm);
no_device:
	fprintf (stderr, "unable initialize capture device: %s\n",
		 snd_strerror(rc));
	return 1;
}
