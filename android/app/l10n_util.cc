/*
 * Copyright 2010, The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "android/app/l10n_util.h"

#include "base/logging.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"

namespace l10n_util {

class JNIHelper {
    public:
        JNIHelper();
        ~JNIHelper();
        string16 getLocalisedString(int message_id);

    private:
        JNIEnv* mEnv;
        jclass mClassRef;
        string16 mMessageCache[ANDROID_L10N_IDS_MESSAGE_COUNT];
};

JNIHelper jniHelper;

JNIHelper::JNIHelper()
    : mEnv(0)
{
}

JNIHelper::~JNIHelper()
{
    if (mEnv) {
        // Because the stored mEnv could be bad during c++ destruction phase
        // since it's thread local.
        JNIEnv* currentEnv = android::GetJNIEnv();
        if (currentEnv)
            currentEnv->DeleteGlobalRef(mClassRef);
    }
}

string16 JNIHelper::getLocalisedString(int message_id)
{
    if (!mEnv) {
        mEnv = android::GetJNIEnv();
        jclass localClass = mEnv->FindClass("android/webkit/L10nUtils");
        mClassRef = static_cast<jclass>(mEnv->NewGlobalRef(localClass));
        mEnv->DeleteLocalRef(localClass);
    }

    if (!mMessageCache[message_id].empty())
        return mMessageCache[message_id];

    DCHECK(mEnv);
    static jmethodID getLocalisedString = mEnv->GetStaticMethodID(mClassRef, "getLocalisedString", "(I)Ljava/lang/String;");
    jstring result = static_cast<jstring>(mEnv->CallStaticObjectMethod(mClassRef, getLocalisedString, message_id));
    string16 str = android::JstringToString16(mEnv, result);
    mEnv->DeleteLocalRef(result);
    mMessageCache[message_id] = str;
    return str;
}

// Implementation of the necessary libchromium l10n utility functions...

string16 GetStringUTF16(int message_id)
{
    return jniHelper.getLocalisedString(message_id);
}

string16 GetStringFUTF16(int message_id, const string16& a, const string16& b, const string16& c)
{
    const string16 str = GetStringUTF16(message_id);
    std::vector<string16> replacements;
    replacements.push_back(a);
    replacements.push_back(b);
    replacements.push_back(c);
    return ReplaceStringPlaceholders(str, replacements, NULL);
}

}
