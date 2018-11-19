/*
 * Copyright (C) 2018 Light Transport Entertainment, Inc.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android/bitmap.h>
#include <android/log.h>
#include <jni.h>
#include <time.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "embree3/rtcore.h"

#define LOG_TAG "libembreerender"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

/* Set to 1 to enable debug log traces. */
#define DEBUG 0

/* Return current time in milliseconds */
static double now_ms(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000. + tv.tv_usec / 1000.;
}

static void fill_render(AndroidBitmapInfo* info, void* pixels, double t) {
	int yy;
	for (yy = 0; yy < info->height; yy++) {
		uint32_t* line = (uint32_t*)pixels;

		int xx;
		for (xx = 0; xx < info->width; xx++) {
			line[xx] = 0;
		}

		// go to next line
		pixels = (char*)pixels + info->stride;
	}
}

/* simple stats management */
typedef struct {
	double renderTime;
	double frameTime;
} FrameStats;

#define MAX_FRAME_STATS 200
#define MAX_PERIOD_MS 1500

typedef struct {
	double firstTime;
	double lastTime;
	double frameTime;

	int firstFrame;
	int numFrames;
	FrameStats frames[MAX_FRAME_STATS];
} Stats;

static void stats_init(Stats* s) {
	s->lastTime = now_ms();
	s->firstTime = 0.;
	s->firstFrame = 0;
	s->numFrames = 0;
}

static void stats_startFrame(Stats* s) { s->frameTime = now_ms(); }

static void stats_endFrame(Stats* s) {
	double now = now_ms();
	double renderTime = now - s->frameTime;
	double frameTime = now - s->lastTime;
	int nn;

	if (now - s->firstTime >= MAX_PERIOD_MS) {
		if (s->numFrames > 0) {
			double minRender, maxRender, avgRender;
			double minFrame, maxFrame, avgFrame;
			int count;

			nn = s->firstFrame;
			minRender = maxRender = avgRender =
			    s->frames[nn].renderTime;
			minFrame = maxFrame = avgFrame =
			    s->frames[nn].frameTime;
			for (count = s->numFrames; count > 0; count--) {
				nn += 1;
				if (nn >= MAX_FRAME_STATS)
					nn -= MAX_FRAME_STATS;
				double render = s->frames[nn].renderTime;
				if (render < minRender) minRender = render;
				if (render > maxRender) maxRender = render;
				double frame = s->frames[nn].frameTime;
				if (frame < minFrame) minFrame = frame;
				if (frame > maxFrame) maxFrame = frame;
				avgRender += render;
				avgFrame += frame;
			}
			avgRender /= s->numFrames;
			avgFrame /= s->numFrames;

			LOGI(
			    "frame/s (avg,min,max) = (%.1f,%.1f,%.1f) "
			    "render time ms (avg,min,max) = (%.1f,%.1f,%.1f)\n",
			    1000. / avgFrame, 1000. / maxFrame,
			    1000. / minFrame, avgRender, minRender, maxRender);
		}
		s->numFrames = 0;
		s->firstFrame = 0;
		s->firstTime = now;
	}

	nn = s->firstFrame + s->numFrames;
	if (nn >= MAX_FRAME_STATS) nn -= MAX_FRAME_STATS;

	s->frames[nn].renderTime = renderTime;
	s->frames[nn].frameTime = frameTime;

	if (s->numFrames < MAX_FRAME_STATS) {
		s->numFrames += 1;
	} else {
		s->firstFrame += 1;
		if (s->firstFrame >= MAX_FRAME_STATS)
			s->firstFrame -= MAX_FRAME_STATS;
	}

	s->lastTime = now;
}

JNIEXPORT void JNICALL
Java_com_example_embreerender_EmbreeRenderView_renderScene(JNIEnv* env,
							   jobject obj,
							   jobject bitmap,
							   jlong time_ms) {
	AndroidBitmapInfo info;
	void* pixels;
	int ret;
	static Stats stats;
	static int init;

	if (!init) {
		// init_tables();
		// stats_init(&stats);
		init = 1;
	}

	if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("Bitmap format is not RGBA_8888 !");
		return;
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	stats_startFrame(&stats);

	/* Now fill the values with a nice little ray tracing result */
	fill_render(&info, pixels, time_ms);

	AndroidBitmap_unlockPixels(env, bitmap);

	stats_endFrame(&stats);
}
