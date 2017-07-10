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
#ifndef  __WIDECOLOR_CTX__
#define  __WIDECOLOR_CTX__

/*
 * CreateWideColorCtx()
 *    Create OpenGL context and Rendering Surface in Display P3 color space
 */
bool CreateWideColorCtx(struct engine* );

/*
 * DestroyWideColorCtx()
 *    Destroy Drawing Surface and OpenGL Context
 */
void DestroyWideColorCtx(struct engine*);

#endif // __WIDECOLOR_CTX__
