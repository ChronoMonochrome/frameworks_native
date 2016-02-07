/*
 * Copyright (C) 2007 The Android Open Source Project
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

#define LOG_TAG "SurfaceControl"

#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <android/native_window.h>

#include <utils/CallStack.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/threads.h>

#include <binder/IPCThreadState.h>

#include <ui/DisplayInfo.h>
#include <ui/GraphicBuffer.h>
#include <ui/Rect.h>

#include <gui_legacy/ISurface.h>
#include <gui_legacy/ISurfaceComposer.h>
#include <gui_legacy/Surface.h>
#include <gui_legacy/SurfaceComposerClient.h>
#include <gui_legacy/SurfaceControl.h>

namespace android {

// ============================================================================
//  SurfaceControl
// ============================================================================

SurfaceControl::SurfaceControl(
        const sp<SurfaceComposerClient>& client, 
        const sp<ISurface>& surface)
    : mClient(client)
{
    if (surface != 0) {
        mSurface = surface->asBinder();
        mGraphicBufferProducer = surface->getSurfaceTexture();
    }
}
        
SurfaceControl::~SurfaceControl()
{
    destroy();
}

void SurfaceControl::destroy()
{
    if (isValid()) {
        mClient->destroySurface(mSurface);
    }
    // clear all references and trigger an IPC now, to make sure things
    // happen without delay, since these resources are quite heavy.
    mClient.clear();
    mSurface.clear();
    mGraphicBufferProducer.clear();
    IPCThreadState::self()->flushCommands();
}

void SurfaceControl::clear() 
{
    // here, the window manager tells us explicitly that we should destroy
    // the surface's resource. Soon after this call, it will also release
    // its last reference (which will call the dtor); however, it is possible
    // that a client living in the same process still holds references which
    // would delay the call to the dtor -- that is why we need this explicit
    // "clear()" call.
    destroy();
}

bool SurfaceControl::isSameSurface(
        const sp<SurfaceControl>& lhs, const sp<SurfaceControl>& rhs) 
{
    if (lhs == 0 || rhs == 0)
        return false;
    return lhs->mSurface == rhs->mSurface;
}

status_t SurfaceControl::setLayerStack(int32_t layerStack) {
    status_t err = validate();
    if (err < 0) return err;
    const sp<SurfaceComposerClient>& client(mClient);
    return client->setLayerStack(mSurface, layerStack);
}
status_t SurfaceControl::setLayer(int32_t layer) {
    status_t err = validate();
    if (err < 0) return err;
    const sp<SurfaceComposerClient>& client(mClient);
    return client->setLayer(mSurface, layer);
}
status_t SurfaceControl::setPosition(int32_t x, int32_t y) {
    status_t err = validate();
    if (err < 0) return err;
    const sp<SurfaceComposerClient>& client(mClient);
    return client->setPosition(mSurface, x, y);
}
status_t SurfaceControl::setSize(uint32_t w, uint32_t h) {
    status_t err = validate();
    if (err < 0) return err;
    const sp<SurfaceComposerClient>& client(mClient);
    return client->setSize(mSurface, w, h);
}
status_t SurfaceControl::hide() {
    status_t err = validate();
    if (err < 0) return err;
    const sp<SurfaceComposerClient>& client(mClient);
    return client->hide(mSurface);
}
status_t SurfaceControl::show() {
    status_t err = validate();
    if (err < 0) return err;
    const sp<SurfaceComposerClient>& client(mClient);
    return client->show(mSurface);
}
status_t SurfaceControl::setFlags(uint32_t flags, uint32_t mask) {
    status_t err = validate();
    if (err < 0) return err;
    const sp<SurfaceComposerClient>& client(mClient);
    return client->setFlags(mSurface, flags, mask);
}
status_t SurfaceControl::setTransparentRegionHint(const Region& transparent) {
    status_t err = validate();
    if (err < 0) return err;
    const sp<SurfaceComposerClient>& client(mClient);
    return client->setTransparentRegionHint(mSurface, transparent);
}
status_t SurfaceControl::setAlpha(float alpha) {
    status_t err = validate();
    if (err < 0) return err;
    const sp<SurfaceComposerClient>& client(mClient);
    return client->setAlpha(mSurface, alpha);
}
status_t SurfaceControl::setMatrix(float dsdx, float dtdx, float dsdy, float dtdy) {
    status_t err = validate();
    if (err < 0) return err;
    const sp<SurfaceComposerClient>& client(mClient);
    return client->setMatrix(mSurface, dsdx, dtdx, dsdy, dtdy);
}
status_t SurfaceControl::setCrop(const Rect& crop) {
    status_t err = validate();
    if (err < 0) return err;
    const sp<SurfaceComposerClient>& client(mClient);
    return client->setCrop(mSurface, crop);
}

status_t SurfaceControl::validate() const
{
    if (mSurface==0 || mClient==0) {
        ALOGE("invalid ISurface (%p) or client (%p)",
                mSurface.get(), mClient.get());
        return NO_INIT;
    }
    return NO_ERROR;
}

status_t SurfaceControl::writeSurfaceToParcel(
        const sp<SurfaceControl>& control, Parcel* parcel)
{
    sp<IGraphicBufferProducer> bp;
    if (control != NULL) {
        bp = control->mGraphicBufferProducer;
    }
    return parcel->writeStrongBinder(bp->asBinder());
}

sp<Surface> SurfaceControl::getSurface() const
{
    Mutex::Autolock _l(mLock);
    if (mSurfaceData == 0) {
        mSurfaceData = new Surface(mGraphicBufferProducer);
    }
    return mSurfaceData;
}

// ----------------------------------------------------------------------------
}; // namespace android
