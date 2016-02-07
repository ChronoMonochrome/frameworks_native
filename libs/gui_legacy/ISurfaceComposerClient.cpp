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

// tag as surfaceflinger
#define LOG_TAG "SurfaceFlinger"

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <ui/Point.h>
#include <ui/Rect.h>

#include <gui_legacy/ISurface.h>
#include <gui_legacy/ISurfaceComposerClient.h>
#include <private/gui_legacy/LayerState.h>

// ---------------------------------------------------------------------------

namespace android {

enum {
    CREATE_SURFACE = IBinder::FIRST_CALL_TRANSACTION,
    DESTROY_SURFACE
};

class BpSurfaceComposerClient : public BpInterface<ISurfaceComposerClient>
{
public:
    BpSurfaceComposerClient(const sp<IBinder>& impl)
        : BpInterface<ISurfaceComposerClient>(impl)
    {
    }

    virtual sp<ISurface> createSurface( const String8& name,
                                        uint32_t w,
                                        uint32_t h,
                                        PixelFormat format,
                                        uint32_t flags)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposerClient::getInterfaceDescriptor());
        data.writeString8(name);
        data.writeInt32(w);
        data.writeInt32(h);
        data.writeInt32(format);
        data.writeInt32(flags);
        remote()->transact(CREATE_SURFACE, data, &reply);
        return interface_cast<ISurface>(reply.readStrongBinder());
    }

    virtual status_t destroySurface(const sp<IBinder>& handle)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposerClient::getInterfaceDescriptor());
        data.writeStrongBinder(handle);
        remote()->transact(DESTROY_SURFACE, data, &reply);
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(SurfaceComposerClient, "android.ui.ISurfaceComposerClient");

// ----------------------------------------------------------------------

status_t BnSurfaceComposerClient::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
     switch(code) {
        case CREATE_SURFACE: {
            CHECK_INTERFACE(ISurfaceComposerClient, data, reply);
            String8 name = data.readString8();
            uint32_t w = data.readInt32();
            uint32_t h = data.readInt32();
            PixelFormat format = data.readInt32();
            uint32_t flags = data.readInt32();
            sp<ISurface> s = createSurface(name, w, h, format, flags);
            reply->writeStrongBinder(s->asBinder());
            return NO_ERROR;
        } break;
        case DESTROY_SURFACE: {
            CHECK_INTERFACE(ISurfaceComposerClient, data, reply);
            reply->writeInt32( destroySurface( data.readStrongBinder() ) );
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
