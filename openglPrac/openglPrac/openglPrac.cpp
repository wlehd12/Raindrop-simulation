#define STB_IMAGE_IMPLEMENTATION

#include <stdlib.h>
#include <GL/glut.h>
#include <stdio.h> 
#include <time.h>
#include <math.h>
#include <string>
#include <stb_image.h>

using namespace std;

#define RAINSIZE 300 
#define dropGen 300
#define TRAILSIZE 20 

int winWidth = 900, winHeight = 600;
time_t t;
float deltatime = 0;
float buttonForce = 0.0;
int alpha = 0;
GLuint texture;
GLuint backgroundTexture;

struct drop {
    float x = 0.0;
    float y = 0.0;
    float radius = 0.0;
    float weight = 0.0;
    float time = 0.0;
    float vel = 0.0;
    float acc = 0.0;
    bool visible = false;
    struct drop* trail[TRAILSIZE];
    drop() {
        for (int i = 0; i < TRAILSIZE; i++) {
            trail[i] = nullptr;
        }
    }
};

drop* rain[RAINSIZE];


void initRain(int i) {
    rain[i] = nullptr;
    delete rain[i];
    rain[i] = new drop;
    rain[i]->x = rand() % (winWidth-20) + 10;
    rain[i]->y = rand() % (winHeight-20) + 10;
    rain[i]->radius = (float)(rand() % 4 + 5);
    rain[i]->weight = pow(rain[i]->radius, 3) * 3.14 * 4 / 3 * 0.006;
    rain[i]->acc = rain[i]->weight * 0.98;
    rain[i]->vel = 0.0f;
    rain[i]->time = (float)(rand() % dropGen) ;
    rain[i]->visible = false;

}

void Image(GLfloat textureID, float a1, float b1, float a2, float b2) {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBegin(GL_QUADS);

    glTexCoord2f(1.0f  , 1.0f );
    glVertex2f(a1, b1);

    glTexCoord2f(0.0f  , 1.0f );
    glVertex2f(a2, b1);

    glTexCoord2f(0.0f  , 0.0f );
    glVertex2f(a2, b2);

    glTexCoord2f(1.0f  , 0.0f );
    glVertex2f(a1, b2);

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}


GLuint createTexture(const std::string& filename) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);


    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);
    }
    else {
        printf("Failed to load texture");
    }
    return textureID;
}


GLuint mixTexture(const std::string& filename1, const std::string& filename2) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width1, height1, nrChannels1;
    int width2, height2, nrChannels2;
    unsigned char* data1 = stbi_load(filename1.c_str(), &width1, &height1, &nrChannels1, 0);
    unsigned char* data2 = stbi_load(filename2.c_str(), &width2, &height2, &nrChannels2, 0);

    if (data1 && data2) {
        int count1 = 0;
        int count2 = 0;
        int count3 = 0;
        for (int j = 0; j < height1; j++) {
            for (int i = 0; i < width1; i++) {
                data1[count1] = data2[count2];
                data1[count1 + 1] = data2[count2 + 1];
                data1[count1 + 2] = data2[count2 + 2];
                // use color texture .rg and calculate alpha
                float distance = sqrt(pow(i - width1 / 2.0f, 2) + pow(j - height1 / 2.0f, 2));
                if (data1[count1 + 3] > 50)
                    data1[count1 + 3] = min(255,(int)(floor(pow(distance,2) / 5)));
                count1 += 4;
                count2 += 3;
                count3 += 3;
            }
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data1);
        stbi_image_free(data2);
    }
    else {
        printf("Failed to load texture");
    }
    return textureID;
}


void initTrails(int k, int i, drop* sourceDrop) {
    if (sourceDrop != nullptr) {
        rain[i]->trail[k] = new drop;
        if (rain[i]->trail[k] != nullptr) {
            rain[i]->trail[k]->x = sourceDrop->x;
            rain[i]->trail[k]->y = sourceDrop->y;
            rain[i]->trail[k]->radius = sourceDrop->radius;
        }
        else {
            rain[i]->trail[k] = nullptr;
            delete rain[i]->trail[k];
        }
    }
    else {
        rain[i]->trail[k] = nullptr;
    }
}

void drawTrails(int i) {
    int beta = min(TRAILSIZE, (int)floor(TRAILSIZE * rain[i]->vel * 0.2));
    if (alpha < beta) {
        initTrails(alpha, i, rain[i]);
        alpha++;
        if (alpha == beta) {
            alpha = 0;
        }
    }

    // Draw trail
    for (int d = 0; d < beta; d++) {
        if (rain[i]->trail[d] != nullptr) {
            Image(texture, rain[i]->trail[d]->x - rain[i]->trail[d]->radius,
                rain[i]->trail[d]->y - rain[i]->trail[d]->radius,
                rain[i]->trail[d]->x + rain[i]->trail[d]->radius,
                rain[i]->trail[d]->y + rain[i]->trail[d]->radius);
            rain[i]->trail[d]->radius *= 0.95; // Shrink
            if (rain[i]->trail[d]->radius < 0.1) {
                rain[i]->trail[d] = nullptr;
                delete rain[i]->trail[d];
            }
        }
    }
}

void Collision(int i, int j) {
    if (rain[i]->visible && rain[j]->visible) {
        float distance = sqrt(pow(rain[i]->x - rain[j]->x, 2) + pow(rain[i]->y - rain[j]->y, 2));
        if (distance < rain[i]->radius + rain[j]->radius) {
            rain[i]->radius += rain[j]->radius / 2;
            rain[i]->weight = pow(rain[i]->radius, 3) * 3.14 * 4 / 3 * 0.006;
            rain[j] = nullptr;
            delete rain[j];
            initRain(j);
        }
    }
}

//라플라스-영(Laplace-Young) 식 :  P(중력) = 2T(표면장력) / R(반지름) 
// 표면장력T = rhρg/2
void ApplyForce(int i) {
    if (rain[i]->visible && rain[i]->weight * 0.98 > 2 * 0.98 * pow(rain[i]->radius, 2) / rain[i]->radius) {
        rain[i]->acc = -sin(rain[i]->vel * deltatime + sin(rain[i]->vel * deltatime + sin(rain[i]->vel * deltatime) * 0.5)) * 0.45;
        if ((rain[i]->weight * 0.98 - 2 * 0.98 * pow(rain[i]->radius, 2) / rain[i]->radius + rain[i]->acc) > 0) {
            rain[i]->vel += (rain[i]->weight * 0.98 - 2 * 0.98 * pow(rain[i]->radius, 2) / rain[i]->radius + rain[i]->acc) * 0.01;
            rain[i]->y -= abs(buttonForce) + rain[i]->vel * 0.1;
            rain[i]->x += buttonForce + (sin(0.02 * rain[i]->y) * pow(sin(0.02 * rain[i]->y), 6)) * 0.005;
            drawTrails(i);
        }
    }
}


void drawDrop(int i) {
    Image(texture, rain[i]->x - rain[i]->radius, rain[i]->y - rain[i]->radius,
        rain[i]->x + rain[i]->radius, rain[i]->y + rain[i]->radius);

    for (int j = 0; j < RAINSIZE; j++) {
        if (i != j) {
            Collision(i, j);
        }
    }
    ApplyForce(i);

    if (rain[i]->y < 0) {
        rain[i] = nullptr;
        delete rain[i];
        initRain(i);
    }
}

void RainAmount() {
    srand((unsigned)time(&t));
    for (int i = 0; i < RAINSIZE; i++) {
        initRain(i);
    }
}

void drawRain() {
    deltatime = fmod(deltatime + 0.01, dropGen );
    for (int i = 0; i < RAINSIZE; i++) {
        rain[i]->visible = (rain[i]->time > deltatime);
        drawDrop(i);
    }
}

void init() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    gluOrtho2D(0.0, winWidth, 0.0, winHeight);
    texture = mixTexture("drop-alpha.png", "clean-background.png");
    backgroundTexture = createTexture("background.png");
}


void background() {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);

    glBegin(GL_QUADS);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(0, 0);

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(winWidth, 0);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(winWidth, winHeight);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(0, winHeight);

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    background();
    drawRain();
    glutSwapBuffers();
    glutPostRedisplay();
    glFlush();
}

void idle() {
    display();
}

void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'q':
    case 'Q':
        exit(0);
    case 'a':
    case 'A':
        buttonForce = -1.0;
        break;
    case 's':
    case 'S':
        buttonForce = 0.0;
        break;
    case 'd':
    case 'D':
        buttonForce = 1.0;
        break;
    case 'r':
    case 'R':
        RainAmount();
        deltatime = 0;
        break;
    }
    glutPostRedisplay();
}



int main(int argc, char** argv) {
    RainAmount();
    printf("%s\n", glGetString(GL_VERSION));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(winWidth, winHeight); // set window position
    glutInitWindowPosition(0, 0); // set window size
    glutCreateWindow("rainDrop");
    glutKeyboardFunc(Keyboard);
    init();
    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}