
package com.atelieryl.wonderdroid.utils;

import com.atelieryl.wonderdroid.WonderSwan;

import android.media.AudioTrack;

public class AudioRunnable implements Runnable {

	private AudioTrack audio;
	
	public AudioRunnable(AudioTrack audio) {
		this.audio = audio;
	}

	@Override
	public void run() {
		android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_MORE_FAVORABLE);
		audio.write(WonderSwan.audiobuffer, 0, WonderSwan.prevSamples * 2 + WonderSwan.samples * 2);
	}
	
}