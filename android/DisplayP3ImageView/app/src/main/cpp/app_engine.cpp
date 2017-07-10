/*
 * Copyright (C) 2017 The Android Open Source Project
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
 *
 */
#include <memory>
#include "app_engine.h"
#include "gltexture.h"

/*
 * Create Rendering Context
 */
bool engine_init_display(struct engine* engine) {

  EnableWelcomeUI(engine);

  bool status = CreateWideColorCtx(engine);
  ASSERT(status, "CreateWideColorContext() failed");

  status = engine->program_.createProgram();
  ASSERT(status, "CreateShaderProgram Failed");

  status = CreateTextures(engine);
  ASSERT(status, "LoadTextures() Failed")

  // Other GL States
  glDisable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, engine->width, engine->height);

  EnableRenderUI(engine);
  return true;
}

/*
 * A quad to view texture
 */
const GLfloat gTriangleVertices[] = {
    -1.f, -1.0f,  0.0f, 1.0f,
    0.0f, -1.0f,  1.0f, 1.0f,
    0.0f,  1.0f,  1.0f, 0.0f,
    -1.0f, 1.0f,  0.0f, 0.0f, };
const GLfloat gTriangleVerticesRight[] = {
    0.0f, -1.0f,  0.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 1.0f,
    1.0f,  1.0f,  1.0f, 0.0f,
    0.0f, 1.0f,  0.0f, 0.0f, };
void engine_draw_frame(struct engine* engine) {
  if (engine->display == NULL) {
    return;
  }

  // Just fill the screen with a color.
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(engine->program_.getProgram());

  glVertexAttribPointer(engine->program_.getAttribLocation(),
                        2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, gTriangleVertices);
  glEnableVertexAttribArray(engine->program_.getAttribLocation());

  glVertexAttribPointer(engine->program_.getAttribLocationTex(),
                        2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, gTriangleVertices + 2);
  glEnableVertexAttribArray(engine->program_.getAttribLocationTex());
  int32_t texIdx = engine->textureIdx_;
  if(engine->renderModeBits_ & RENDERING_P3) {
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, engine->textures_[texIdx]->P3TexId());
    glUniform1i(engine->program_.getSamplerLoc(), 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }
  if (engine->renderModeBits_ & RENDERING_SRGB) {
    glVertexAttribPointer(engine->program_.getAttribLocation(),
                          2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4,
                          gTriangleVerticesRight);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, engine->textures_[texIdx]->SRGBATexId());
    glUniform1i(engine->program_.getSamplerLoc(), 1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }

  eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void engine_term_display(struct engine* engine) {

  engine->animating = 0;

  if(engine->display == EGL_NO_DISPLAY)
    return;

  DestroyWideColorCtx(engine);

  glDeleteProgram(engine->program_.getProgram());
  DeleteTextures(engine);
}

/**
 * Process the next input event.
 */
int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
  struct engine* engine = (struct engine*)app->userData;

  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    engine_ProcessInputEvent(engine, event);

    return 1;
  }
  return 0;
}

/**
 * Process the next main command.
 */
void engine_handle_cmd(struct android_app* app, int32_t cmd) {
  struct engine* engine = (struct engine*)app->userData;
  switch (cmd) {
    case APP_CMD_SAVE_STATE:
      break;
    case APP_CMD_INIT_WINDOW:
      // The window is being shown, get it ready.
      if (engine->app->window != NULL) {
        engine_init_display(engine);
        engine_draw_frame(engine);
        engine->animating = 1;
      }
      break;
    case APP_CMD_TERM_WINDOW:
      // The window is being hidden or closed, clean it up.
      engine_term_display(engine);
      break;
    case APP_CMD_GAINED_FOCUS:
      // When our app gains focus, we start monitoring the accelerometer.
      if (engine->accelerometerSensor != NULL) {
        ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                       engine->accelerometerSensor);
        // We'd like to get 60 events per second (in us).
        ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                       engine->accelerometerSensor,
                                       (1000L/60)*1000);
        engine->animating = 1;
      }
      break;
    case APP_CMD_LOST_FOCUS:
      // When our app loses focus, we stop monitoring the accelerometer.
      // This is to avoid consuming battery while not being used.
      if (engine->accelerometerSensor != NULL) {
        ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                                        engine->accelerometerSensor);
      }
      // Also stop animating.
      engine->animating = 0;
      engine_draw_frame(engine);
      break;
  }
}

/*
 * Initialize the application engine
 */
engine::engine() :
    animating(0),
    textureIdx_(0) {
  textures_.resize(0);
  renderModeBits_ = RENDERING_P3 | RENDERING_SRGB;
  context = EGL_NO_CONTEXT;
  surface = EGL_NO_SURFACE;
  display = EGL_NO_DISPLAY;
}