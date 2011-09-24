
package uk.org.cardboardbox.wonderdroid.utils;

import uk.org.cardboardbox.wonderdroid.WonderSwan;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.media.AudioTrack;
import android.os.SystemClock;
import android.util.Log;
import android.view.SurfaceHolder;

public class EmuThread extends Thread {

	public static interface Renderer {
		public void update(boolean skip);
		public void render (Canvas c, boolean frameskip, boolean showFps, String fpsString);
	}

	private Renderer renderer;

	private static final boolean debug = false;
	private static final String TAG = EmuThread.class.getSimpleName();
	private static final int TARGETFRAMETIME = 1000 / 37;

	private boolean mIsRunning = false;
	private boolean isPaused = false;
	private boolean showFps = false;

	

	private SurfaceHolder mSurfaceHolder;

	private Canvas c;

	private long frameStart;
	private long frameEnd;
	private long frametime;
	private long holdtime;
	private long averageFrameTime = TARGETFRAMETIME;

	public EmuThread (Renderer renderer) {
		
		this.renderer = renderer;

	}

	public void setSurfaceHolder (SurfaceHolder sh) {
		mSurfaceHolder = sh;
	}

	public void pause () {
		isPaused = true;
		WonderSwan.audio.pause();
	}

	public void unpause () {
		isPaused = false;
		if (WonderSwan.audio.getState() == AudioTrack.PLAYSTATE_PAUSED) {
			WonderSwan.audio.play();
		}
	}

	@Override
	public void run () {

		while (mSurfaceHolder == null) {
			SystemClock.sleep(20);
		}

		// benchmark
		/*
		 * long start = System.currentTimeMillis(); for (int frame = 0; frame < 60; frame++) {
		 * 
		 * c = null; try { c = mSurfaceHolder.lockCanvas(); synchronized (mSurfaceHolder) { c.drawARGB(0x00, frame, frame, frame); }
		 * } finally { if (c != null) { mSurfaceHolder.unlockCanvasAndPost(c); } }
		 * 
		 * } float fps = (float)(1f / (((System.currentTimeMillis() - start) / 1000f) / 60f)); Log.d(TAG, String.format("%f fps",
		 * fps)); //
		 */

		// WonderSwan.audio.play();

		while (mIsRunning) {

			if (isPaused) {
				SystemClock.sleep(TARGETFRAMETIME);

			} else {

				frameStart = System.currentTimeMillis();
				// boolean skip = framecounter % mustSkipFrames == 0 || lastFrame > thisFrame + TARGETFRAMETIME;

				boolean skip = false;

				renderer.update(skip);
				
				c = null;
				try {
					c = mSurfaceHolder.lockCanvas();
					synchronized (mSurfaceHolder) {
						renderer.render(c, skip, showFps, fpsString);

					}
				} finally {
					if (c != null) {
						mSurfaceHolder.unlockCanvasAndPost(c);
					}
				}

				frameEnd = System.currentTimeMillis();
				frametime = frameEnd - frameStart;
				averageFrameTime = (averageFrameTime + frametime) / 2;
				updateFPSString();

				if (frametime < TARGETFRAMETIME) {
					SystemClock.sleep(TARGETFRAMETIME - frametime);
				}

			}

		}

		// WonderSwan.audio.stop();
		
		synchronized (this) {
			notifyAll();
		}

	}

	public boolean isRunning () {
		return mIsRunning;
	}

	public void setRunning () {
		mIsRunning = true;
	}

	public void clearRunning () {
		mIsRunning = false;
	}

	private String fpsString = new String();

	private void updateFPSString () {
		fpsString = String.format("%d frametime", averageFrameTime);
	}

	public void showFps (boolean show) {
		showFps = show;
	}

}
