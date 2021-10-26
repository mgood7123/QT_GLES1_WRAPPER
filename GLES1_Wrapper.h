#ifndef GLES1_WRAPPER_H
#define GLES1_WRAPPER_H

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QStack>

#include "GLUTesselator/src/tess.h"

class GLES1_Wrapper
{
    static const char * vertex_shader;
    static const char * fragment_shader;
    QOpenGLContext * context;
    QOpenGLFunctions *gles2;
    QOpenGLExtraFunctions *gles3;
    QOpenGLShaderProgram shader;

    QStack<QMatrix4x4> stack_GL_PROJECTION_MATRIX;
    QStack<QMatrix4x4> stack_GL_MODELVIEW_MATRIX;
    QStack<QMatrix4x4> stack_GL_TEXTURE_MATRIX;
    QStack<QMatrix4x4> stack_GL_COLOR_MATRIX;

    int projectionUniform;
    int modelViewUniform;
    int normalUniform;

    GLenum matrixMode;

    QMatrix4x4 & getCurrentMatrix();
    QVector3D currentNormal;

    QList<float> vertexData;
    GLsizei vertexCount;

    GLfloat vertex_x;
    GLfloat vertex_y;
    GLfloat vertex_z;
    GLfloat vertex_w;

    // ortho
    GLint vertex_x_int;
    GLint vertex_y_int;
    GLint vertex_z_int;
    GLint vertex_w_int;

    // the default color is white
    GLfloat color_red = 0;
    GLfloat color_green = 0;
    GLfloat color_blue = 0;
    GLfloat color_alpha = 1;


    GLenum primitiveMode;
    bool begin;

    QMatrix4x4 toMatrix(const GLfloat * m);
    QMatrix4x4 toMatrix(const GLdouble * m);

public:

    void glBegin(GLenum mode);
    void glEnd();

    GLES1_Wrapper(QOpenGLContext * context);

    void glOrtho(	GLdouble left,
        GLdouble right,
        GLdouble bottom,
        GLdouble top,
        GLdouble nearVal,
        GLdouble farVal);

    void gluOrtho2D(	GLdouble left,
        GLdouble right,
        GLdouble bottom,
        GLdouble top);

    void gluPerspective(	GLdouble fovy,
        GLdouble aspect,
        GLdouble zNear,
        GLdouble zFar);

    void glFrustum(	GLdouble left,
        GLdouble right,
        GLdouble bottom,
        GLdouble top,
        GLdouble nearVal,
        GLdouble farVal);

    void glMatrixMode(GLenum mode);

    GLenum getMatrixMode();

    void glLoadIdentity();

    void glPushMatrix();
    void glPopMatrix();

    void glLoadMatrixd(const GLdouble * m);
    void glLoadMatrixf(const GLfloat * m);

    void glMultMatrixd(const GLdouble * m);
    void glMultMatrixf(const GLfloat * m);

    void glLoadTransposeMatrixd(	const GLdouble * m);
    void glLoadTransposeMatrixf(	const GLfloat * m);

    void glMultTransposeMatrixd(const GLdouble * m);
    void glMultTransposeMatrixf(const GLfloat * m);

    void glTranslated(GLdouble x, GLdouble y, GLdouble z);
    void glTranslatef(GLfloat x, GLfloat y, GLfloat z);

    void glScaled(GLdouble x, GLdouble y, GLdouble z);
    void glScalef(GLfloat x, GLfloat y, GLfloat z);

    void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
    void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

    void glNormal3b(	GLbyte nx,
        GLbyte ny,
        GLbyte nz);

    void glNormal3d(	GLdouble nx,
        GLdouble ny,
        GLdouble nz);

    void glNormal3f(	GLfloat nx,
        GLfloat ny,
        GLfloat nz);

    void glNormal3i(	GLint nx,
        GLint ny,
        GLint nz);

    void glNormal3s(	GLshort nx,
        GLshort ny,
        GLshort nz);

    void glVertex2s(	GLshort x,
        GLshort y);

    void glVertex2i(	GLint x,
        GLint y);

    void glVertex2f(	GLfloat x,
        GLfloat y);

    void glVertex2d(	GLdouble x,
        GLdouble y);

    void glVertex3s(	GLshort x,
        GLshort y,
        GLshort z);

    void glVertex3i(	GLint x,
        GLint y,
        GLint z);

    void glVertex3f(	GLfloat x,
        GLfloat y,
        GLfloat z);

    void glVertex3d(	GLdouble x,
        GLdouble y,
        GLdouble z);

    void glVertex4s(	GLshort x,
        GLshort y,
        GLshort z,
        GLshort w);

    void glVertex4i(	GLint x,
        GLint y,
        GLint z,
        GLint w);

    void glVertex4f(	GLfloat x,
        GLfloat y,
        GLfloat z,
        GLfloat w);

    void glVertex4d(	GLdouble x,
        GLdouble y,
        GLdouble z,
        GLdouble w);

    void glVertex2sv(	const GLshort * v);

    void glVertex2iv(	const GLint * v);

    void glVertex2fv(	const GLfloat * v);

    void glVertex2dv(	const GLdouble * v);

    void glVertex3sv(	const GLshort * v);

    void glVertex3iv(	const GLint * v);

    void glVertex3fv(	const GLfloat * v);

    void glVertex3dv(	const GLdouble * v);

    void glVertex4sv(	const GLshort * v);

    void glVertex4iv(	const GLint * v);

    void glVertex4fv(	const GLfloat * v);

    void glVertex4dv(	const GLdouble * v);

    void glColor3b(	GLbyte red,
        GLbyte green,
        GLbyte blue);

    void glColor3s(	GLshort red,
        GLshort green,
        GLshort blue);

    void glColor3i(	GLint red,
        GLint green,
        GLint blue);

    void glColor3f(	GLfloat red,
        GLfloat green,
        GLfloat blue);

    void glColor3d(	GLdouble red,
        GLdouble green,
        GLdouble blue);

    void glColor3ub(	GLubyte red,
        GLubyte green,
        GLubyte blue);

    void glColor3us(	GLushort red,
        GLushort green,
        GLushort blue);

    void glColor3ui(	GLuint red,
        GLuint green,
        GLuint blue);

    void glColor4b(	GLbyte red,
        GLbyte green,
        GLbyte blue,
        GLbyte alpha);

    void glColor4s(	GLshort red,
        GLshort green,
        GLshort blue,
        GLshort alpha);

    void glColor4i(	GLint red,
        GLint green,
        GLint blue,
        GLint alpha);

    void glColor4f(	GLfloat red,
        GLfloat green,
        GLfloat blue,
        GLfloat alpha);

    void glColor4d(	GLdouble red,
        GLdouble green,
        GLdouble blue,
        GLdouble alpha);

    void glColor4ub(	GLubyte red,
        GLubyte green,
        GLubyte blue,
        GLubyte alpha);

    void glColor4us(	GLushort red,
        GLushort green,
        GLushort blue,
        GLushort alpha);

    void glColor4ui(	GLuint red,
        GLuint green,
        GLuint blue,
        GLuint alpha);

    void glColor3bv(	const GLbyte * v);

    void glColor3sv(	const GLshort * v);

    void glColor3iv(	const GLint * v);

    void glColor3fv(	const GLfloat * v);

    void glColor3dv(	const GLdouble * v);

    void glColor3ubv(	const GLubyte * v);

    void glColor3usv(	const GLushort * v);

    void glColor3uiv(	const GLuint * v);

    void glColor4bv(	const GLbyte * v);

    void glColor4sv(	const GLshort * v);

    void glColor4iv(	const GLint * v);

    void glColor4fv(	const GLfloat * v);

    void glColor4dv(	const GLdouble * v);

    void glColor4ubv(	const GLubyte * v);

    void glColor4usv(	const GLushort * v);

    void glColor4uiv(	const GLuint * v);
};

#endif // GLES1_WRAPPER_H
