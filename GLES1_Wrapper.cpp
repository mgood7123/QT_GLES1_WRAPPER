#include "GLES1_Wrapper.h"

#include <QOpenGLBuffer>

const char * GLES1_Wrapper::vertex_shader = R"(
layout (location = 0) in vec4 vertex_position;
layout (location = 1) in vec4 vertex_color;
uniform mat4 projection;
uniform mat4 modelView;
uniform vec3 normal;

out vec4 fragment_in_color;

void main()
{
    gl_Position = projection * modelView * vertex_position;
    fragment_in_color = vertex_color;
}
)";

const char * GLES1_Wrapper::fragment_shader = R"(
// input
in highp vec4 fragment_in_color;

// output
out highp vec4 FragColor;

// code
void main() {
    FragColor = fragment_in_color;
}
)";

void GLES1_Wrapper::glBegin(GLenum mode)
{
    if (begin) return;
    vertexCount = 0;
    vertexData.clear();
    vertexData.squeeze();
    primitiveMode = mode;
    begin = true;
}

void GLES1_Wrapper::glEnd()
{
    if (!begin) return;
    shader.bind();
    GLuint VAO;
    GLuint VBO;
    gles3->glGenVertexArrays(1, &VAO);
    gles2->glGenBuffers(1, &VBO);
    gles3->glBindVertexArray(VAO);
    gles2->glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    qDebug() << "set vertex buffer data to" << vertexData;
    gles2->glBufferData(GL_ARRAY_BUFFER, vertexData.length() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    int position_components = 4;
    int color_components = 4;
    int vertex_position_of_position = 0;
    int vertex_position_of_color = position_components * sizeof(float);
    int stride = position_components + color_components;

    GLboolean isNormalizationEnabled = glIsEnabled(GL_NORMALIZE);

    // position attribute
    gles2->glVertexAttribPointer(0, position_components, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)vertex_position_of_position);
    gles2->glEnableVertexAttribArray(0);

    // color attribute
    gles2->glVertexAttribPointer(1, color_components, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)vertex_position_of_color);
    gles2->glEnableVertexAttribArray(1);

    shader.setUniformValue(projectionUniform, stack_GL_PROJECTION_MATRIX.last());
    shader.setUniformValue(modelViewUniform, stack_GL_MODELVIEW_MATRIX.last());
    shader.setUniformValue(normalUniform, currentNormal);

    if (primitiveMode == GL_QUADS) {
        // calculate quad elements index array
        QList<int> elements;
        auto c = vertexData.length();
        qsizetype i = 0;
        int p = 0;
        auto increment = stride * 4;
        while(i < c) {
            int i0 = p++;
            int i1 = p++;
            int i2 = p++;
            int i3 = p++;
            elements.append({
                i0, i1, i2,
                i0, i2, i3
            });
            i += increment;
        }

        unsigned int EBO;
        gles2->glGenBuffers(1, &EBO);

        gles2->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//        qDebug() << "set element buffer data to" << elements;
        gles2->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements.data(), GL_STATIC_DRAW);

        gles2->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        auto elementCount = elements.length();
        qDebug() << "drawing" << elementCount << "elements";
        gles2->glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);
    } else {
        qDebug() << "drawing" << vertexCount << "vertices";
        gles2->glDrawArrays(primitiveMode, 0, vertexCount);
    }

    gles3->glDeleteVertexArrays(1, &VAO);
    gles2->glDeleteBuffers(1, &VBO);
    shader.release();

    // clean up
    vertexCount = 0;
    vertexData.clear();
    vertexData.squeeze();

    begin = false;
}

QMatrix4x4 &GLES1_Wrapper::getCurrentMatrix() {
    switch (matrixMode) {
    case GL_PROJECTION:
        return stack_GL_PROJECTION_MATRIX.last();
    case GL_MODELVIEW:
        return stack_GL_MODELVIEW_MATRIX.last();
    case GL_TEXTURE:
        return stack_GL_TEXTURE_MATRIX.last();
    case GL_COLOR:
        return stack_GL_COLOR_MATRIX.last();
    default:
        qFatal("unknown matrix mode");
    }
}

QMatrix4x4 GLES1_Wrapper::toMatrix(const GLfloat *m) {
    return {
        m[0],  m[1],  m[2],  m[3],
                m[4],  m[5],  m[6],  m[7],
                m[8],  m[9],  m[10], m[11],
                m[12], m[13], m[14], m[15]
    };
}

QMatrix4x4 GLES1_Wrapper::toMatrix(const GLdouble *m) {
    return {
        static_cast<float>(m[0]),  static_cast<float>(m[1]),  static_cast<float>(m[2]),  static_cast<float>(m[3]),
                static_cast<float>(m[4]),  static_cast<float>(m[5]),  static_cast<float>(m[6]),  static_cast<float>(m[7]),
                static_cast<float>(m[8]),  static_cast<float>(m[9]),  static_cast<float>(m[10]), static_cast<float>(m[11]),
                static_cast<float>(m[12]), static_cast<float>(m[13]), static_cast<float>(m[14]), static_cast<float>(m[15])
    };
}

GLES1_Wrapper::GLES1_Wrapper(QOpenGLContext * context) : context(context) {
    gles2 = context->functions();
    gles3 = context->extraFunctions();
    glMatrixMode(GL_MODELVIEW_MATRIX);
    stack_GL_PROJECTION_MATRIX.push(QMatrix4x4());
    stack_GL_MODELVIEW_MATRIX.push(QMatrix4x4());
    stack_GL_TEXTURE_MATRIX.push(QMatrix4x4());
    stack_GL_COLOR_MATRIX.push(QMatrix4x4());
    currentNormal = {0, 0, 1};

    if (!shader.isLinked()) {
        QByteArray v = vertex_shader;
        QByteArray f = fragment_shader;

        // Any number representing a version of the language a compiler does not support
        // will cause an error to be generated.

        if (context->isOpenGLES()) {
            // android supports OpenGL ES 3.0 since android 4.3 (kitkat)

            // android supports OpenGL ES 3.1 since android 5.0 (lollipop)
            //  New functionality in OpenGL ES 3.1 includes:
            //   Compute shaders
            //   Independent vertex and fragment shaders
            //   Indirect draw commands

            // android supports OpenGL ES 3.2 since android 6.0 (marshmellow), possibly 7.0 (naugat)

            // OpenGL ES 3.0 (#version 300 es)
            // OpenGL ES 3.1 (#version 310 es)
            // OpenGL ES 3.2 (#version 320 es)

            v.prepend(QByteArrayLiteral("#version 300 es\n"));
            f.prepend(QByteArrayLiteral("#version 300 es\n"));
        } else {
            // OpenGL 3.3 (GLSL #version 330)
            v.prepend(QByteArrayLiteral("#version 330\n"));
            f.prepend(QByteArrayLiteral("#version 330\n"));
        }
        if (!shader.addShaderFromSourceCode(QOpenGLShader::Vertex, v)) {
            qFatal("OPENGL SHADER VERTEX SHADER COMPILATION FAILED");
        }
        if (!shader.addShaderFromSourceCode(QOpenGLShader::Fragment, f)) {
            qFatal("OPENGL SHADER FRAGMENT SHADER COMPILATION FAILED");
        }
        if (!shader.link()) {
            qFatal("OPENGL SHADER LINK FAILED");
        }
    }

    shader.bind();
    projectionUniform = shader.uniformLocation("projection");
    modelViewUniform = shader.uniformLocation("modelView");
    normalUniform = shader.uniformLocation("normal");
}

void GLES1_Wrapper::glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal)
{
    getCurrentMatrix().ortho(left, right, bottom, top, nearVal, farVal);
}

void GLES1_Wrapper::gluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top)
{
    glOrtho(left, right, bottom, top, -1, 1);
}

void GLES1_Wrapper::gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    getCurrentMatrix().perspective(fovy, aspect, zNear, zFar);
}

void GLES1_Wrapper::glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal)
{
    getCurrentMatrix().frustum(left, right, bottom, top, nearVal, farVal);
}

void GLES1_Wrapper::glMatrixMode(GLenum mode) {
    matrixMode = mode;
}

GLenum GLES1_Wrapper::getMatrixMode() {
    return matrixMode;
}

void GLES1_Wrapper::glLoadIdentity() {
    getCurrentMatrix().setToIdentity();
}

void GLES1_Wrapper::glPushMatrix()
{
    switch (matrixMode) {
    case GL_PROJECTION:
        stack_GL_PROJECTION_MATRIX.push(QMatrix4x4(stack_GL_PROJECTION_MATRIX.last()));
    case GL_MODELVIEW:
        stack_GL_MODELVIEW_MATRIX.push(QMatrix4x4(stack_GL_MODELVIEW_MATRIX.last()));
    case GL_TEXTURE:
        stack_GL_TEXTURE_MATRIX.push(QMatrix4x4(stack_GL_TEXTURE_MATRIX.last()));
    case GL_COLOR:
        stack_GL_COLOR_MATRIX.push(QMatrix4x4(stack_GL_COLOR_MATRIX.last()));
    default:
        qFatal("unknown matrix mode");
    }
}

void GLES1_Wrapper::glPopMatrix()
{
    switch (matrixMode) {
    case GL_PROJECTION:
        if (stack_GL_PROJECTION_MATRIX.length() > 1) {
            stack_GL_PROJECTION_MATRIX.removeLast();
        }
    case GL_MODELVIEW:
        if (stack_GL_MODELVIEW_MATRIX.length() > 1) {
            stack_GL_MODELVIEW_MATRIX.removeLast();
        }
    case GL_TEXTURE:
        if (stack_GL_TEXTURE_MATRIX.length() > 1) {
            stack_GL_TEXTURE_MATRIX.removeLast();
        }
    case GL_COLOR:
        if (stack_GL_COLOR_MATRIX.length() > 1) {
            stack_GL_COLOR_MATRIX.removeLast();
        }
    default:
        qFatal("unknown matrix mode");
    }
}

void GLES1_Wrapper::glLoadMatrixd(const GLdouble *m)
{
    getCurrentMatrix() = toMatrix(m);
}

void GLES1_Wrapper::glLoadMatrixf(const GLfloat *m)
{
    getCurrentMatrix() = toMatrix(m);
}

void GLES1_Wrapper::glMultMatrixd(const GLdouble *m)
{
    getCurrentMatrix() *= toMatrix(m);
}

void GLES1_Wrapper::glMultMatrixf(const GLfloat *m)
{
    getCurrentMatrix() *= toMatrix(m);
}

void GLES1_Wrapper::glLoadTransposeMatrixd(const GLdouble *m)
{
    getCurrentMatrix() = toMatrix(m).transposed();
}

void GLES1_Wrapper::glLoadTransposeMatrixf(const GLfloat *m)
{
    getCurrentMatrix() = toMatrix(m).transposed();
}

void GLES1_Wrapper::glMultTransposeMatrixd(const GLdouble *m)
{
    auto & current = getCurrentMatrix();
    current = (current * toMatrix(m)).transposed();
}

void GLES1_Wrapper::glMultTransposeMatrixf(const GLfloat *m)
{
    auto & current = getCurrentMatrix();
    current = (current * toMatrix(m)).transposed();
}

void GLES1_Wrapper::glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
    getCurrentMatrix().translate(x, y, z);
}

void GLES1_Wrapper::glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    getCurrentMatrix().translate(x, y, z);
}

void GLES1_Wrapper::glScaled(GLdouble x, GLdouble y, GLdouble z)
{
    getCurrentMatrix().scale(x, y, z);
}

void GLES1_Wrapper::glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    getCurrentMatrix().scale(x, y, z);
}

void GLES1_Wrapper::glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    getCurrentMatrix().rotate(angle, x, y, z);
}

void GLES1_Wrapper::glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    getCurrentMatrix().rotate(angle, x, y, z);
}

void GLES1_Wrapper::glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
    currentNormal = {static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz)};
}

void GLES1_Wrapper::glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
    currentNormal = {static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz)};
}

void GLES1_Wrapper::glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    currentNormal = {static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz)};
}

void GLES1_Wrapper::glNormal3i(GLint nx, GLint ny, GLint nz)
{
    currentNormal = {static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz)};
}

void GLES1_Wrapper::glNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
    currentNormal = {static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz)};
}

void GLES1_Wrapper::glVertex2s(GLshort x, GLshort y)
{
    vertex_x_int = x;
    vertex_y_int = y;
}

void GLES1_Wrapper::glVertex2i(GLint x, GLint y)
{
    vertex_x_int = x;
    vertex_y_int = y;
}

void GLES1_Wrapper::glVertex2f(GLfloat x, GLfloat y)
{
    vertex_x = x;
    vertex_y = y;
}

void GLES1_Wrapper::glVertex2d(GLdouble x, GLdouble y)
{
    vertex_x = x;
    vertex_y = y;
}

void GLES1_Wrapper::glVertex3s(GLshort x, GLshort y, GLshort z)
{
    vertex_x_int = x;
    vertex_y_int = y;
    vertex_z_int = z;
}

void GLES1_Wrapper::glVertex3i(GLint x, GLint y, GLint z)
{
    vertex_x_int = x;
    vertex_y_int = y;
    vertex_z_int = z;
}

void GLES1_Wrapper::glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    vertexData.append({x, y, z, 1, color_red, color_blue, color_green, color_alpha});
    vertexCount++;
}

void GLES1_Wrapper::glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    vertexData.append({static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 1, color_red, color_blue, color_green, color_alpha});
    vertexCount++;
}

void GLES1_Wrapper::glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    vertex_x_int = x;
    vertex_y_int = y;
    vertex_z_int = z;
    vertex_w_int = z;
}

void GLES1_Wrapper::glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
    vertex_x_int = x;
    vertex_y_int = y;
    vertex_z_int = z;
    vertex_w_int = z;
}

void GLES1_Wrapper::glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    vertexData.append({x, y, z, w, color_red, color_blue, color_green, color_alpha});
    vertexCount++;
}

void GLES1_Wrapper::glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    vertexData.append({static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w), color_red, color_blue, color_green, color_alpha});
    vertexCount++;
}

void GLES1_Wrapper::glVertex2sv(const GLshort *v)
{
    glVertex2s(v[0], v[1]);
}

void GLES1_Wrapper::glVertex2iv(const GLint *v)
{
    glVertex2i(v[0], v[1]);
}

void GLES1_Wrapper::glVertex2fv(const GLfloat *v)
{
    glVertex2f(v[0], v[1]);
}

void GLES1_Wrapper::glVertex2dv(const GLdouble *v)
{
    glVertex2d(v[0], v[1]);
}

void GLES1_Wrapper::glVertex3sv(const GLshort *v)
{
    glVertex3s(v[0], v[1], v[2]);
}

void GLES1_Wrapper::glVertex3iv(const GLint *v)
{
    glVertex3i(v[0], v[1], v[2]);
}

void GLES1_Wrapper::glVertex3fv(const GLfloat *v)
{
    glVertex3f(v[0], v[1], v[2]);
}

void GLES1_Wrapper::glVertex3dv(const GLdouble *v)
{
    glVertex3d(v[0], v[1], v[2]);
}

void GLES1_Wrapper::glVertex4sv(const GLshort *v)
{
    glVertex4s(v[0], v[1], v[2], v[3]);
}

void GLES1_Wrapper::glVertex4iv(const GLint *v)
{
    glVertex4i(v[0], v[1], v[2], v[3]);
}

void GLES1_Wrapper::glVertex4fv(const GLfloat *v)
{
    glVertex4f(v[0], v[1], v[2], v[3]);
}

void GLES1_Wrapper::glVertex4dv(const GLdouble *v)
{
    glVertex4d(v[0], v[1], v[2], v[3]);
}

void GLES1_Wrapper::glColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
    color_red = static_cast<float>(red) / 255.0f;
    color_green = static_cast<float>(green) / 255.0f;
    color_blue = static_cast<float>(blue) / 255.0f;
}

void GLES1_Wrapper::glColor3s(GLshort red, GLshort green, GLshort blue)
{
    color_red = static_cast<float>(red) / 255.0f;
    color_green = static_cast<float>(green) / 255.0f;
    color_blue = static_cast<float>(blue) / 255.0f;
}

void GLES1_Wrapper::glColor3i(GLint red, GLint green, GLint blue)
{
    color_red = static_cast<float>(red) / 255.0f;
    color_green = static_cast<float>(green) / 255.0f;
    color_blue = static_cast<float>(blue) / 255.0f;
}

void GLES1_Wrapper::glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    color_red = red;
    color_green = green;
    color_blue = blue;
}

void GLES1_Wrapper::glColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
    color_red = red;
    color_green = green;
    color_blue = blue;
}

void GLES1_Wrapper::glColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    color_red = static_cast<float>(red) / 255.0f;
    color_green = static_cast<float>(green) / 255.0f;
    color_blue = static_cast<float>(blue) / 255.0f;
}

void GLES1_Wrapper::glColor3us(GLushort red, GLushort green, GLushort blue)
{
    color_red = static_cast<float>(red) / 255.0f;
    color_green = static_cast<float>(green) / 255.0f;
    color_blue = static_cast<float>(blue) / 255.0f;
}

void GLES1_Wrapper::glColor3ui(GLuint red, GLuint green, GLuint blue)
{
    color_red = static_cast<float>(red) / 255.0f;
    color_green = static_cast<float>(green) / 255.0f;
    color_blue = static_cast<float>(blue) / 255.0f;
}

void GLES1_Wrapper::glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    color_red = static_cast<float>(red) / 255.0f;
    color_green = static_cast<float>(green) / 255.0f;
    color_blue = static_cast<float>(blue) / 255.0f;
    color_alpha = static_cast<float>(alpha) / 255.0f;
}

void GLES1_Wrapper::glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    color_red = static_cast<float>(red) / 255.0f;
    color_green = static_cast<float>(green) / 255.0f;
    color_blue = static_cast<float>(blue) / 255.0f;
    color_alpha = static_cast<float>(alpha) / 255.0f;
}

void GLES1_Wrapper::glColor4i(GLint red, GLint green, GLint blue, GLint alpha)
{
    color_red = static_cast<float>(red) / 255.0f;
    color_green = static_cast<float>(green) / 255.0f;
    color_blue = static_cast<float>(blue) / 255.0f;
    color_alpha = static_cast<float>(alpha) / 255.0f;
}

void GLES1_Wrapper::glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    color_red = red;
    color_green = green;
    color_blue = blue;
    color_alpha = alpha;
}

void GLES1_Wrapper::glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    color_red = red;
    color_green = green;
    color_blue = blue;
    color_alpha = alpha;
}

void GLES1_Wrapper::glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    color_red = static_cast<float>(red) / 255.0f;
    color_green = static_cast<float>(green) / 255.0f;
    color_blue = static_cast<float>(blue) / 255.0f;
    color_alpha = static_cast<float>(alpha) / 255.0f;
}

void GLES1_Wrapper::glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    color_red = static_cast<float>(red) / 255.0f;
    color_green = static_cast<float>(green) / 255.0f;
    color_blue = static_cast<float>(blue) / 255.0f;
    color_alpha = static_cast<float>(alpha) / 255.0f;
}

void GLES1_Wrapper::glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    color_red = static_cast<float>(red) / 255.0f;
    color_green = static_cast<float>(green) / 255.0f;
    color_blue = static_cast<float>(blue) / 255.0f;
    color_alpha = static_cast<float>(alpha) / 255.0f;
}

void GLES1_Wrapper::glColor3bv(const GLbyte *v)
{
    glColor3b(v[0], v[1], v[2]);
}

void GLES1_Wrapper::glColor3sv(const GLshort *v)
{
    glColor3s(v[0], v[1], v[2]);
}

void GLES1_Wrapper::glColor3iv(const GLint *v)
{
    glColor3i(v[0], v[1], v[2]);
}

void GLES1_Wrapper::glColor3fv(const GLfloat *v)
{
    glColor3f(v[0], v[1], v[2]);
}

void GLES1_Wrapper::glColor3dv(const GLdouble *v)
{
    glColor3d(v[0], v[1], v[2]);
}

void GLES1_Wrapper::glColor3ubv(const GLubyte *v)
{
    glColor3ub(v[0], v[1], v[2]);
}

void GLES1_Wrapper::glColor3usv(const GLushort *v)
{
    glColor3us(v[0], v[1], v[2]);
}

void GLES1_Wrapper::glColor3uiv(const GLuint *v)
{
    glColor3ui(v[0], v[1], v[2]);
}

void GLES1_Wrapper::glColor4bv(const GLbyte *v)
{
    glColor4ub(v[0], v[1], v[2], v[3]);
}

void GLES1_Wrapper::glColor4sv(const GLshort *v)
{
    glColor4s(v[0], v[1], v[2], v[3]);
}

void GLES1_Wrapper::glColor4iv(const GLint *v)
{
    glColor4i(v[0], v[1], v[2], v[3]);
}

void GLES1_Wrapper::glColor4fv(const GLfloat *v)
{
    glColor4f(v[0], v[1], v[2], v[3]);
}

void GLES1_Wrapper::glColor4dv(const GLdouble *v)
{
    glColor4d(v[0], v[1], v[2], v[3]);
}

void GLES1_Wrapper::glColor4ubv(const GLubyte *v)
{
    glColor4us(v[0], v[1], v[2], v[3]);
}

void GLES1_Wrapper::glColor4usv(const GLushort *v)
{
    glColor4us(v[0], v[1], v[2], v[3]);
}

void GLES1_Wrapper::glColor4uiv(const GLuint *v)
{
    glColor4ui(v[0], v[1], v[2], v[3]);
}
