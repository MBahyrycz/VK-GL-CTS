/*-------------------------------------------------------------------------
 * drawElements Quality Program OpenGL ES 2.0 Module
 * -------------------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
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
 *//*!
 * \file
 * \brief Rbo state query tests.
 *//*--------------------------------------------------------------------*/

#include "es2fRboStateQueryTests.hpp"
#include "glsStateQueryUtil.hpp"
#include "es2fApiCase.hpp"
#include "gluRenderContext.hpp"
#include "glwEnums.hpp"
#include "glwFunctions.hpp"
#include "deRandom.hpp"
#include "deMath.h"

using namespace glw; // GLint and other GL types
using deqp::gls::StateQueryUtil::StateQueryMemoryWriteGuard;

namespace deqp
{
namespace gles2
{
namespace Functional
{
namespace
{

void checkRenderbufferComponentSize(tcu::TestContext &testCtx, glu::CallLogWrapper &gl, int r, int g, int b, int a,
                                    int d, int s)
{
    using tcu::TestLog;

    const int referenceSizes[] = {r, g, b, a, d, s};
    const GLenum paramNames[]  = {GL_RENDERBUFFER_RED_SIZE,   GL_RENDERBUFFER_GREEN_SIZE, GL_RENDERBUFFER_BLUE_SIZE,
                                  GL_RENDERBUFFER_ALPHA_SIZE, GL_RENDERBUFFER_DEPTH_SIZE, GL_RENDERBUFFER_STENCIL_SIZE};

    DE_STATIC_ASSERT(DE_LENGTH_OF_ARRAY(referenceSizes) == DE_LENGTH_OF_ARRAY(paramNames));

    for (int ndx = 0; ndx < DE_LENGTH_OF_ARRAY(referenceSizes); ++ndx)
    {
        if (referenceSizes[ndx] == -1)
            continue;

        StateQueryMemoryWriteGuard<GLint> state;
        gl.glGetRenderbufferParameteriv(GL_RENDERBUFFER, paramNames[ndx], &state);

        if (!state.verifyValidity(testCtx))
            return;

        if (state < referenceSizes[ndx])
        {
            testCtx.getLog() << TestLog::Message << "// ERROR: Expected greater or equal to " << referenceSizes[ndx]
                             << "; got " << state << TestLog::EndMessage;
            if (testCtx.getTestResult() == QP_TEST_RESULT_PASS)
                testCtx.setTestResult(QP_TEST_RESULT_FAIL, "got invalid value");
        }
    }
}

void checkIntEquals(tcu::TestContext &testCtx, GLint got, GLint expected)
{
    using tcu::TestLog;

    if (got != expected)
    {
        testCtx.getLog() << TestLog::Message << "// ERROR: Expected " << expected << "; got " << got
                         << TestLog::EndMessage;
        if (testCtx.getTestResult() == QP_TEST_RESULT_PASS)
            testCtx.setTestResult(QP_TEST_RESULT_FAIL, "got invalid value");
    }
}

void checkRenderbufferParam(tcu::TestContext &testCtx, glu::CallLogWrapper &gl, GLenum pname, GLenum reference)
{
    StateQueryMemoryWriteGuard<GLint> state;
    gl.glGetRenderbufferParameteriv(GL_RENDERBUFFER, pname, &state);

    if (state.verifyValidity(testCtx))
        checkIntEquals(testCtx, state, reference);
}

class RboSizeCase : public ApiCase
{
public:
    RboSizeCase(Context &context, const char *name, const char *description) : ApiCase(context, name, description)
    {
    }

    void test(void)
    {
        de::Random rnd(0xabcdef);

        GLuint renderbufferID = 0;
        glGenRenderbuffers(1, &renderbufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbufferID);
        expectError(GL_NO_ERROR);

        checkRenderbufferParam(m_testCtx, *this, GL_RENDERBUFFER_WIDTH, 0);
        checkRenderbufferParam(m_testCtx, *this, GL_RENDERBUFFER_HEIGHT, 0);
        expectError(GL_NO_ERROR);

        const int numIterations = 30;
        for (int i = 0; i < numIterations; ++i)
        {
            const GLint w = rnd.getInt(0, 128);
            const GLint h = rnd.getInt(0, 128);

            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, w, h);
            expectError(GL_NO_ERROR);

            checkRenderbufferParam(m_testCtx, *this, GL_RENDERBUFFER_WIDTH, w);
            checkRenderbufferParam(m_testCtx, *this, GL_RENDERBUFFER_HEIGHT, h);
        }

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
        glDeleteRenderbuffers(1, &renderbufferID);
    }
};

class RboInternalFormatCase : public ApiCase
{
public:
    RboInternalFormatCase(Context &context, const char *name, const char *description)
        : ApiCase(context, name, description)
    {
    }

    void test(void)
    {
        GLuint renderbufferID = 0;
        glGenRenderbuffers(1, &renderbufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbufferID);
        expectError(GL_NO_ERROR);

        checkRenderbufferParam(m_testCtx, *this, GL_RENDERBUFFER_INTERNAL_FORMAT, GL_RGBA4);
        expectError(GL_NO_ERROR);

        const GLenum requiredColorformats[] = {GL_RGBA4, GL_RGB5_A1, GL_RGB565};

        for (int ndx = 0; ndx < DE_LENGTH_OF_ARRAY(requiredColorformats); ++ndx)
        {
            glRenderbufferStorage(GL_RENDERBUFFER, requiredColorformats[ndx], 1, 1);
            expectError(GL_NO_ERROR);

            checkRenderbufferParam(m_testCtx, *this, GL_RENDERBUFFER_INTERNAL_FORMAT, requiredColorformats[ndx]);
        }

        glDeleteRenderbuffers(1, &renderbufferID);
    }
};

class RboComponentSizeColorCase : public ApiCase
{
public:
    RboComponentSizeColorCase(Context &context, const char *name, const char *description)
        : ApiCase(context, name, description)
    {
    }

    void test(void)
    {
        GLuint renderbufferID = 0;
        glGenRenderbuffers(1, &renderbufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbufferID);
        expectError(GL_NO_ERROR);

        checkRenderbufferComponentSize(m_testCtx, *this, 0, 0, 0, 0, 0, 0);
        expectError(GL_NO_ERROR);

        const struct ColorFormat
        {
            GLenum internalFormat;
            int bitsR, bitsG, bitsB, bitsA;
        } requiredColorFormats[] = {
            {GL_RGBA4, 4, 4, 4, 4},
            {GL_RGB5_A1, 5, 5, 5, 1},
            {GL_RGB565, 5, 6, 5, 0},
        };

        for (int ndx = 0; ndx < DE_LENGTH_OF_ARRAY(requiredColorFormats); ++ndx)
        {
            glRenderbufferStorage(GL_RENDERBUFFER, requiredColorFormats[ndx].internalFormat, 1, 1);
            expectError(GL_NO_ERROR);

            checkRenderbufferComponentSize(m_testCtx, *this, requiredColorFormats[ndx].bitsR,
                                           requiredColorFormats[ndx].bitsG, requiredColorFormats[ndx].bitsB,
                                           requiredColorFormats[ndx].bitsA, -1, -1);
        }

        glDeleteRenderbuffers(1, &renderbufferID);
    }
};

class RboComponentSizeDepthCase : public ApiCase
{
public:
    RboComponentSizeDepthCase(Context &context, const char *name, const char *description)
        : ApiCase(context, name, description)
    {
    }

    void test(void)
    {
        using tcu::TestLog;

        GLuint renderbufferID = 0;
        glGenRenderbuffers(1, &renderbufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbufferID);
        expectError(GL_NO_ERROR);

        const struct DepthFormat
        {
            GLenum internalFormat;
            int dbits;
            int sbits;
        } requiredDepthFormats[] = {
            {GL_DEPTH_COMPONENT16, 16, 0},
        };

        for (int ndx = 0; ndx < DE_LENGTH_OF_ARRAY(requiredDepthFormats); ++ndx)
        {
            glRenderbufferStorage(GL_RENDERBUFFER, requiredDepthFormats[ndx].internalFormat, 1, 1);
            expectError(GL_NO_ERROR);

            checkRenderbufferComponentSize(m_testCtx, *this, -1, -1, -1, -1, requiredDepthFormats[ndx].dbits,
                                           requiredDepthFormats[ndx].sbits);
        }

        // STENCIL_INDEX8 is required, in that case sBits >= 8
        {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, 1, 1);
            expectError(GL_NO_ERROR);

            StateQueryMemoryWriteGuard<GLint> state;
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_STENCIL_SIZE, &state);

            if (state.verifyValidity(m_testCtx) && state < 8)
            {
                m_testCtx.getLog() << TestLog::Message << "// ERROR: Expected greater or equal to 8; got " << state
                                   << TestLog::EndMessage;
                if (m_testCtx.getTestResult() == QP_TEST_RESULT_PASS)
                    m_testCtx.setTestResult(QP_TEST_RESULT_FAIL, "got invalid value");
            }
        }

        glDeleteRenderbuffers(1, &renderbufferID);
    }
};

} // namespace

RboStateQueryTests::RboStateQueryTests(Context &context) : TestCaseGroup(context, "rbo", "Rbo State Query tests")
{
}

void RboStateQueryTests::init(void)
{
    addChild(new RboSizeCase(m_context, "renderbuffer_size", "RENDERBUFFER_WIDTH and RENDERBUFFER_HEIGHT"));
    addChild(new RboInternalFormatCase(m_context, "renderbuffer_internal_format", "RENDERBUFFER_INTERNAL_FORMAT"));
    addChild(new RboComponentSizeColorCase(m_context, "renderbuffer_component_size_color", "RENDERBUFFER_x_SIZE"));
    addChild(new RboComponentSizeDepthCase(m_context, "renderbuffer_component_size_depth", "RENDERBUFFER_x_SIZE"));
}

} // namespace Functional
} // namespace gles2
} // namespace deqp
