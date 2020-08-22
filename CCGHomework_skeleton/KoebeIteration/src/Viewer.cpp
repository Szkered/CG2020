#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef MAC_OS
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif // MAC_OS

#include "bmp/RgbImage.h"
#include "viewer/Arcball.h" /*  Arc Ball  Interface         */
#include "HoleFiller.h"
#include "CircularSlitMap.h"

using namespace MeshLib;

/* window width and height */
int g_win_width, g_win_height;
int g_button;
int g_startx, g_starty;
int g_shade_flag = 0;
bool g_show_mesh = true;
bool g_show_uv = false;
bool g_show_boundary = true;

/* rotation quaternion and translation vector for the object */
CQrot g_obj_rot(0, 0, 1, 0);
CPoint g_obj_trans(0, 0, 0);

/* arcball object */
CArcball g_arcball;

/* global g_mesh */
// CHodgeDecompositionMesh* g_domain_mesh = NULL;
std::vector<CHodgeDecompositionMesh *> g_meshes; // exact holomorphic 1-forms
std::vector<CHodgeDecompositionMesh *> h_meshes; // closed, non-exact holomorphic 1-forms

// CHodgeDecomposition g_mapper;
int g_show_index = 0;

int g_texture_flag = 2;
/* texture id and g_image */
GLuint g_texture_name;
RgbImage g_image;

/*! initialize bitmap g_image texture */
void initializeBmpTexture()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &g_texture_name);
    glBindTexture(GL_TEXTURE_2D, g_texture_name);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    int ImageWidth = g_image.GetNumCols();
    int ImageHeight = g_image.GetNumRows();
    GLubyte *ptr = (GLubyte *)g_image.ImageData();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ImageWidth, ImageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, ptr);

    if (g_texture_flag == 1)
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    else if (g_texture_flag == 2)
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
}

/*! setup the object, transform from the world to the object coordinate system */
void setupObject(void)
{
    double rot[16];

    glTranslated(g_obj_trans[0], g_obj_trans[1], g_obj_trans[2]);
    g_obj_rot.convert(rot);
    glMultMatrixd((GLdouble *)rot);
}

/*! the eye is always fixed at world z = +5 */
void setupEye(void)
{
    glLoadIdentity();
    gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
}

/*! setup light */
void setupLight()
{
    GLfloat lightOnePosition[4] = {0, 0, 1, 0};
    GLfloat lightTwoPosition[4] = {0, 0, -1, 0};
    glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);
    glLightfv(GL_LIGHT2, GL_POSITION, lightTwoPosition);
}

/*! draw g_mesh */
void drawMesh(CHodgeDecompositionMesh *pMesh)
{
    glEnable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, g_texture_name);
    if (g_texture_flag > 0)
        glEnable(GL_TEXTURE_2D);

    glLineWidth(1.0);
    for (CHodgeDecompositionMesh::MeshFaceIterator fiter(pMesh); !fiter.end(); ++fiter)
    {
        glBegin(GL_POLYGON);
        CHodgeDecompositionFace *pF = *fiter;
        std::vector<CPoint> points;
        std::vector<CPoint> normals;
        std::vector<CPoint> rgbs;
        std::vector<CPoint2> uvs;

        for (CHodgeDecompositionMesh::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
        {
            CHodgeDecompositionVertex *pV = *fviter;
            if (pV->id() == 21347)
            {
                int a = 0;
            }
            CPoint &p = pV->point();
            CPoint2 &uv = pV->uv();
            CPoint &rgb = pV->rgb();
            CPoint n;
            switch (g_shade_flag)
            {
            case 0:
                n = pF->normal();
                break;
            case 1:
                n = pV->normal();
                break;
            }
            normals.push_back(n);
            rgbs.push_back(rgb);
            points.push_back(p);
            uvs.push_back(uv);
        }

        bool filled_face = false;
        for (size_t i = 0; i < 3; i++)
        {
            if ((rgbs[i] - CPoint(1, 0, 0)).norm() < 0.05)
            {
                filled_face = true;
                break;
            }
        }
        if (filled_face)
        {
            for (size_t i = 0; i < 3; i++)
            {
                rgbs[i] = CPoint(1, 0, 0);
                normals[i] = CPoint(0, 0, 1);
            }
        }

        for (size_t i = 0; i < 3; i++)
        {
            CPoint n = normals[i];
            CPoint2 uv = uvs[i];
            CPoint rgb = rgbs[i];
            CPoint p = points[i];

            glNormal3d(n[0], n[1], n[2]);
            glTexCoord2d(uv[0], uv[1]);
            glColor3f(rgb[0], rgb[1], rgb[2]);
            glVertex3d(p[0], p[1], p[2]);
        }
        glEnd();
    }
}

/*! draw uv mesh */
void drawUv(CHodgeDecompositionMesh *pMesh)
{
    glEnable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, g_texture_name);
    if (g_texture_flag > 0)
        glEnable(GL_TEXTURE_2D);

    glLineWidth(1.0);
    glColor3f(229.0 / 255.0, 162.0 / 255.0, 141.0 / 255.0);
    for (CHodgeDecompositionMesh::MeshFaceIterator fiter(pMesh); !fiter.end(); ++fiter)
    {
        glBegin(GL_POLYGON);
        CHodgeDecompositionFace *pF = *fiter;

        std::vector<CPoint> normals;
        std::vector<CPoint> rgbs;
        std::vector<CPoint2> uvs;

        for (CHodgeDecompositionMesh::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
        {
            CHodgeDecompositionVertex *pV = *fviter;
            CPoint2 &uv = pV->uv();
            CPoint &rgb = pV->rgb();
            CPoint n;
            switch (g_shade_flag)
            {
            case 0:
                n = pF->normal();
                break;
            case 1:
                n = pV->normal();
                break;
            }
            normals.push_back(n);
            rgbs.push_back(rgb);
            uvs.push_back(uv);
        }

        bool filled_face = false;
        for (size_t i = 0; i < 3; i++)
        {
            if ((rgbs[i] - CPoint(1, 0, 0)).norm() < 0.05)
            {
                filled_face = true;
                break;
            }
        }
        if (filled_face)
        {
            for (size_t i = 0; i < 3; i++)
            {
                rgbs[i] = CPoint(1, 0, 0);
                normals[i] = CPoint(0, 0, 1);
            }
        }

        for (size_t i = 0; i < 3; i++)
        {
            CPoint n = normals[i];
            CPoint2 uv = uvs[i];
            CPoint rgb = rgbs[i];

            glNormal3d(n[0], n[1], n[2]);
            glTexCoord2d(uv[0], uv[1]);
            glColor3f(rgb[0], rgb[1], rgb[2]);
            glVertex3d(uv[0], uv[1], 0);
        }
        glEnd();
    }
}

/*! draw boundary
 *  mode: 1, mesh; 2, uv
 */
void drawBoundary(int mode, CHodgeDecompositionMesh *pMesh)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(3.0);
    glColor3f(0, 0, 1);
    glBegin(GL_LINES);
    for (CHodgeDecompositionMesh::MeshEdgeIterator eit(pMesh); !eit.end(); ++eit)
    {
        CHodgeDecompositionMesh::CEdge *pE = *eit;
        if (!pE->boundary())
            continue;

        CHodgeDecompositionMesh::CVertex *pA = pMesh->edgeVertex1(pE);
        CHodgeDecompositionMesh::CVertex *pB = pMesh->edgeVertex2(pE);

        if (mode == 1) // draw mesh
        {
            CPoint &a = pA->point();
            CPoint &b = pB->point();
            glVertex3d(a[0], a[1], a[2]);
            glVertex3d(b[0], b[1], b[2]);
        }
        else if (mode == 2) // draw uv
        {
            CPoint2 &a = pA->uv();
            CPoint2 &b = pB->uv();
            glVertex3d(a[0], a[1], 0);
            glVertex3d(b[0], b[1], 0);
        }
    }
    glEnd();
}

/*! display call back function
 */
void display()
{
    /* clear frame buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setupLight();
    /* transform from the eye coordinate system to the world system */
    setupEye();
    glPushMatrix();
    /* transform from the world to the ojbect coordinate system */
    setupObject();

    /* draw the mesh */
    switch (g_texture_flag)
    {
    case 0:
        glDisable(GL_TEXTURE_2D);
        break;
    case 1:
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        break;
    case 2:
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        break;
    }

    /* draw the mesh */
    if (g_show_mesh)
    {
        if (g_show_boundary)
            drawBoundary(1, g_meshes[0]);
        drawMesh(g_meshes[0]);
    }
    if (g_show_uv)
    {
        if (g_show_boundary)
            drawBoundary(2, g_meshes[0]);
        drawUv(g_meshes[0]);
    }

    glPopMatrix();
    glutSwapBuffers();
}

/*! Called when a "resize" event is received by the window. */
void reshape(int w, int h)
{
    float ar;

    g_win_width = w;
    g_win_height = h;

    ar = (float)(w) / h;
    glViewport(0, 0, w, h); /* Set Viewport */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(40.0, /* field of view in degrees */
                   ar,   /* aspect ratio */
                   0.1,  /* Z near */
                   100.0 /* Z far */);

    glMatrixMode(GL_MODELVIEW);

    glutPostRedisplay();
}

/*! helper function to remind the user about commands, hot keys */
void help()
{
    printf("\n");
    printf("1  -  Show or hide mesh\n");
    printf("2  -  Show or hide uv\n");
    printf("w  -  Wireframe Display\n");
    printf("f  -  Flat Shading \n");
    printf("s  -  Smooth Shading\n");
    printf("t  -  Texture rendering mode\n");
    printf("b  -  Show or hide boundary\n");
    printf("?  -  Help Information\n");
    printf("esc - Quit\n");
}

/*! Keyboard call back function */
void keyBoard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '1':
        // Show or hide mesh
        g_show_mesh = !g_show_mesh;
        break;
    case '2':
        // Show or hide uv
        g_show_uv = !g_show_uv;
        break;
    case 'f':
        // Flat Shading
        glPolygonMode(GL_FRONT, GL_FILL);
        g_shade_flag = 0;
        break;
    case 's':
        // Smooth Shading
        glPolygonMode(GL_FRONT, GL_FILL);
        g_shade_flag = 1;
        break;
    case 'w':
        // Wireframe mode
        glPolygonMode(GL_FRONT, GL_LINE);
        break;
    case 't':
        // Texture rendering mode
        g_texture_flag = (g_texture_flag + 1) % 3;
        break;
    case 'b':
        // Show or hide boundary
        g_show_boundary = !g_show_boundary;
        break;
    case '?':
        help();
        break;
    case 27:
        exit(0);
        break;
    }
    glutPostRedisplay();
}

/*! setup GL states */
void setupGLstate()
{
    GLfloat lightOneColor[] = {1, 1, 1, 1.0};
    GLfloat globalAmb[] = {.1, .1, .1, 1};
    GLfloat lightOnePosition[] = {.0, 0.0, 1.0, 1.0};
    GLfloat lightTwoPosition[] = {.0, 0.0, -1.0, 1.0};

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 0.0);
    // glClearColor(0.35, 0.53, 0.70, 0);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOneColor);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, lightOneColor);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);
    glLightfv(GL_LIGHT2, GL_POSITION, lightTwoPosition);

    const GLfloat specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64.0f);

    GLfloat mat_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat mat_diffuse[] = {0.01f, 0.01f, 0.01f, 1.0f};
    GLfloat mat_specular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat mat_shininess[] = {32};

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
}

/*! mouse click call back function */
void mouseClick(int button, int state, int x, int y)
{
    /* set up an arcball around the Eye's center
    switch y coordinates to right handed system  */

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        g_button = GLUT_LEFT_BUTTON;
        g_arcball = CArcball(g_win_width, g_win_height, x - g_win_width / 2, g_win_height - y - g_win_height / 2);
    }

    if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
    {
        g_startx = x;
        g_starty = y;
        g_button = GLUT_MIDDLE_BUTTON;
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        g_startx = x;
        g_starty = y;
        g_button = GLUT_RIGHT_BUTTON;
    }
    return;
}

/*! mouse motion call back function */
void mouseMove(int x, int y)
{
    CPoint trans;
    CQrot rot;

    /* rotation, call g_arcball */
    if (g_button == GLUT_LEFT_BUTTON)
    {
        rot = g_arcball.update(x - g_win_width / 2, g_win_height - y - g_win_height / 2);
        g_obj_rot = rot * g_obj_rot;
        glutPostRedisplay();
    }

    /*xy translation */
    if (g_button == GLUT_MIDDLE_BUTTON)
    {
        double scale = 10. / g_win_height;
        trans = CPoint(scale * (x - g_startx), scale * (g_starty - y), 0);
        g_startx = x;
        g_starty = y;
        g_obj_trans = g_obj_trans + trans;
        glutPostRedisplay();
    }

    /* zoom in and out */
    if (g_button == GLUT_RIGHT_BUTTON)
    {
        double scale = 10. / g_win_height;
        trans = CPoint(0, 0, scale * (g_starty - y));
        g_startx = x;
        g_starty = y;
        g_obj_trans = g_obj_trans + trans;
        glutPostRedisplay();
    }
}

void initOpenGL(int argc, char *argv[])
{
    /* glut stuff */
    glutInit(&argc, argv); /* Initialize GLUT */
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Mesh Viewer"); /* Create window with given title */
    glViewport(0, 0, 600, 600);

    glutDisplayFunc(display); /* Set-up callback functions */
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMove);
    glutKeyboardFunc(keyBoard);
    setupGLstate();
    initializeBmpTexture();

    glutMainLoop(); /* Start GLUT event-processing loop */
}

/*! main function for viewer
 */
int main(int argc, char *argv[])
{
    std::srand((unsigned)time(NULL));

    if (strcmp(argv[1], "-view") == 0)
    {
        std::string input_mesh_name(argv[2]);
        if (!strutil::endsWith(input_mesh_name, ".m"))
        {
            printf("Usage: %s -view input.m texture_image\n", argv[0]);
            return EXIT_FAILURE;
        }

        using M = CHodgeDecompositionMesh;
        M *pMesh = new M;
        pMesh->read_m(input_mesh_name.c_str());
        g_meshes.push_back(pMesh);

        normalizeMesh(pMesh);
        computeNormal(pMesh);

        // load texture
        g_image.LoadBmpFile(argv[3]);
    }

    if (strcmp(argv[1], "-fill_hole") == 0)
    {

        std::string original_mesh_name(argv[2]);
        if (!strutil::endsWith(original_mesh_name, ".m"))
        {
            printf("Usage: %s -fill_hole original_mesh.m filled_mesh.m\n", argv[0]);
            return EXIT_FAILURE;
        }

        std::string filled_mesh_name(argv[3]);
        if (!strutil::endsWith(filled_mesh_name, ".m"))
        {
            printf("Usage: %s -fill_hole original_mesh.m filled_mesh.m\n", argv[0]);
            return EXIT_FAILURE;
        }

        CDTMesh original_mesh;
        original_mesh.read_m(original_mesh_name.c_str());
        fill_hole(original_mesh, filled_mesh_name);

        return EXIT_SUCCESS;
    }

    if (argc < 3)
    {
        printf("Usage: %s -circular_slit_map input.m output.m texture_bmp\n", argv[0]);
        help();
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-circular_slit_map") == 0)
    {
        std::string input_mesh_name(argv[2]);
        if (!strutil::endsWith(input_mesh_name, ".m"))
        {
            printf("Usage: %s -circular_slit_map input.m output_mesh.m texture_bmp\n", argv[0]);
            return EXIT_FAILURE;
        }
        std::string output_mesh_name(argv[3]);
        if (!strutil::endsWith(output_mesh_name, ".m"))
        {
            printf("Usage: %s -circular_slit_map  input.m output_mesh.m texture_bmp\n", argv[0]);
            return EXIT_FAILURE;
        }

        MeshLib::calc_holo_1_form_open_mesh(input_mesh_name, g_meshes, h_meshes, output_mesh_name);
        // load texture
        g_image.LoadBmpFile(argv[4]);
    }

    if (strcmp(argv[1], "-punch_hole") == 0)
    {
        std::string original_mesh_name(argv[2]);
        if (!strutil::endsWith(original_mesh_name, ".m"))
        {
            printf("Usage: %s -punch_hole original_mesh.m filled_mesh.m hole_id output_mesh.m\n", argv[0]);
            return EXIT_FAILURE;
        }

        std::string filled_mesh_name(argv[3]);
        if (!strutil::endsWith(filled_mesh_name, ".m"))
        {
            printf("Usage: %s -punch_hole original_mesh.m filled_mesh.m hole_id output_mesh.m\n", argv[0]);
            return EXIT_FAILURE;
        }

        std::string punched_mesh_name(argv[4]);
        if (!strutil::endsWith(punched_mesh_name, ".m"))
        {
            printf("Usage: %s -punch_hole original_mesh.m filled_mesh.m hole_id output_mesh.m\n", argv[0]);
            return EXIT_FAILURE;
        }

        int hole = atoi(argv[5]);

        CDTMesh original_mesh;
        original_mesh.read_m(original_mesh_name.c_str());
        CDTMesh filled_mesh;
        filled_mesh.read_m(filled_mesh_name.c_str());
        punch_hole(original_mesh, filled_mesh, punched_mesh_name, hole);

        return EXIT_SUCCESS;
    }

    initOpenGL(argc, argv);

    return EXIT_SUCCESS;
}
