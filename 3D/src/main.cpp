// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"

// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>

// Linear Algebra Library
#include <Eigen/Core>
#include <Eigen/Geometry>

// Timer
#include <chrono>

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <cstdlib>
using namespace std;



 Program program;


VertexBufferObject VBO;



 bool initial_s = true;
 int ob = -1;


Eigen::Vector3f hitsimage(0.0, 0.0, 0.0);
int ini_length = 480;

bool findposofmousewindow(GLFWwindow *window){

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    GLuint v;
    glReadPixels(xpos, ini_length - ypos - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &v);
    if(v > 0){
        ob = v;
        return true;
    }
    return false;
}
Eigen::Matrix4f zooming(float zoom){

    Eigen::Matrix4f zmatrix(4,4);

    zmatrix <<
            zoom, 0, 0, 0, 0, zoom, 0, 0, 0, 0, zoom, 0, 0, 0, 0, 1;

    return zmatrix;
}

Eigen::Vector3f changeDirection(0.0, 1.0, 0.0);


bool scene = false;



Eigen::Matrix4f matrixforScene(4,4);
int trackerformovement = 0;
bool trackingmouse = false;
bool coloring = false;

Eigen::Matrix4f cammat(Eigen::Vector3f changeDirection, Eigen::Vector3f location, Eigen::Vector3f hitsimage)
{

    Eigen::Vector3f sn = changeDirection.normalized();

    Eigen::Vector3f c = (location - hitsimage).normalized();
   

    Eigen::Vector3f a = (sn.cross(c)).normalized();
    
    Eigen::Vector3f b = c.cross(a);


    Eigen::Matrix4f  transmodified = Eigen::Matrix4f::Identity();
    transmodified(2, 3) = -location.z();

    transmodified(0, 3) = -location.x();

    transmodified(1, 3) = -location.y();

    Eigen::Matrix4f  modifiedrot = Eigen::Matrix4f::Identity();
    modifiedrot(0, 2) = a.z();
    modifiedrot(1, 2) = b.z();

    modifiedrot(2, 2) = c.z();

    modifiedrot(0, 0) = a.x();

    modifiedrot(1, 0) = b.x();

    modifiedrot(2, 0) = c.x();
    modifiedrot(0, 1) = a.y();


    modifiedrot(1, 1) = b.y();


    modifiedrot(2, 1) = c.y();



    return modifiedrot * transmodified;
}

enum Do
{
    Inser,
    Trans,
};
#define PI 3.14

Eigen::Matrix4f modifiedperp(float ar, float maxZcoor, float angle, float closerZcoordinate)
{
    float a = tan((angle*PI/180) /2);

    Eigen::Matrix4f newmat = Eigen::Matrix4f::Zero();

    newmat(2, 2) = -(maxZcoor + closerZcoordinate) / (maxZcoor - closerZcoordinate);
    newmat(1, 1) = 1/(a);

    newmat(0, 0) = 1/(ar * a);

    newmat(2, 3) = - (2 * maxZcoor * closerZcoordinate) / (maxZcoor - closerZcoordinate);

    newmat(3, 2) = -1;

    return newmat;
}
Do typeofchange;


Eigen::Matrix4f viewO(float a, float b, float c, float d, float e, float f){
    Eigen::Matrix4f v_mat_new = Eigen::Matrix4f::Identity();
    v_mat_new(0, 0) = 2 / (b - a);
    v_mat_new(1, 1) = 2 / (d - c);
    v_mat_new(2, 2) = -2 / (f - e);
    v_mat_new(0, 3) = -(b + a) / (b - a);
    v_mat_new(1, 3) = -(d + c) / (d - c);
    v_mat_new(2, 3) = -(f + e) / (f - e);

    return v_mat_new;
}

Eigen::MatrixXf shadingchoices;


Eigen::Matrix4f zaroundzoom(double ang_value){

    Eigen::Matrix4f matforz(4,4);

    matforz <<
    cos(ang_value*PI/180), -sin(ang_value*PI/180), 0, 0, sin(ang_value*PI/180), cos(ang_value*PI/180), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;

    return matforz;
}

vector<unsigned int> First;
enum Thing
{
    cube,
    bunnyimage,
    cubewithbumps
};
float lookfora = 0.0;


Eigen::Matrix4f moving(float x, float y, float z){

    Eigen::Matrix4f moving(4,4);

    moving <<
    1, 0, 0, x, 0, 1, 0, y, 0, 0, 1, z, 0, 0, 0, 1;

    return moving;
}
float lookforb = 0.0;

Eigen::Matrix4f moving(Eigen::Vector3f moving_changed_mat){

    Eigen::Matrix4f moving(4,4);

    moving <<
    1, 0, 0, moving_changed_mat.x(), 0, 1, 0, moving_changed_mat.y(), 0, 0, 1, moving_changed_mat.z(), 0, 0, 0, 1;

    return moving;
}

Eigen::MatrixXf vet;

class info {
public:
    unsigned int main;
    Thing name;
    unsigned int limpoint;
    unsigned int changelim;
    unsigned int pointchange;
    unsigned int change_v;
    Eigen::Vector3f inside;

    info(){};

    info(unsigned int main, Thing name, unsigned int limpoint, unsigned int changelim, unsigned int pointchange, unsigned int change_v, Eigen::Vector3f inside){
        this->main = main;
        this->name = name;
        this->limpoint = limpoint;
        this->changelim = changelim;
        this->pointchange = pointchange;
        this->change_v = change_v;
        this->inside = inside;
    };
};

enum DiffShades
{
    shadingwireframe,
    Shadeflat,
    shadingphong
};

Eigen::MatrixXf coli;

bool loadMeshFromFile(string filename, Eigen::Vector3f &objectCenter) {

    ifstream file;

    int j;
    int oldvetcount = vet.cols();

    Eigen::Vector3f vertexSum = Eigen::Vector3f::Zero();

    file.open ("../../data/"+filename);
    if (!file.is_open()) {
        cout << "Looking at the data folder for the off files" << endl;
        file.open ("../data/"+filename);
        if(!file.is_open()){
            cout << "This is an incorrect off file" << endl;
            return false;
        }
    }
    int cornerTracker;
    int linetracker = 0;
    std::string e_sente;
    int sent = 0;

    while (std::getline (file, e_sente)) {

        std::stringstream wordStream (e_sente);

        std::string efw;
        int nw = 0;

        if(sent == 1){
            while (wordStream >> efw) {
                if(nw == 0){
                    cornerTracker = stoi(efw);
                } else if(nw == 1) {
                    j = stoi(efw);
                }
                nw++;
            }

            vet.conservativeResize(3, oldvetcount + cornerTracker);
            coli.conservativeResize(3, oldvetcount + cornerTracker); //can take this off
        }

        else if(sent > 1 && sent < cornerTracker + 2){
            while (wordStream >> efw) {
                if (nw < 3) {
                    vet(nw, oldvetcount + sent -2) = stof(efw);
                }
                if(nw == 2) {
                    vertexSum = vertexSum + vet.col(oldvetcount + sent -2);
                }
                nw++;
            }
        }
        else if(sent == 0) {
            while (wordStream >> efw) {
                if(efw != "OFF") {
                    return false;
                }
                nw++;
            }
        }  else {
            while (wordStream >> efw) {
                if(sent == cornerTracker + 2 && nw == 0){
                    linetracker = stoi(efw);
                }
                if (nw > 0 && nw < linetracker + 1) {
                    First.push_back(oldvetcount + stoi(efw));
                }
                nw++;
            }
        }
        sent++;
    }
    objectCenter = vertexSum / cornerTracker;
    file.close ();
    return true;
}
Eigen::MatrixXf bary;
DiffShades rendering;
class occ {
public:
    unsigned int main;
    info pic_ob;
    Eigen::Matrix4f imageOg;
    Eigen::Matrix4f imageOgchanged;
    Eigen::Matrix4f origstart;
    Eigen::Matrix4f origstartChanged;
    Eigen::Vector3f Shades;

    occ(unsigned int main, info pic_ob, Eigen::Matrix4f imageOg, Eigen::Matrix4f origstart, Eigen::Vector3f Shades){
        this->main = main;
        this->pic_ob = pic_ob;
        this->imageOg = imageOg;
        this->imageOgchanged = Eigen::Matrix4f::Identity();
        this->origstart = origstart;
        this->origstartChanged = Eigen::Matrix4f::Identity();
        this->Shades = Shades;
    };

    Eigen::Matrix4f getterforImageOg() {
        return this->imageOgchanged * this->imageOg;
    };

    Eigen::Matrix4f getterfororigstartChanged() {
        return this->origstartChanged * this->origstart;
    };
};


list<occ> listocc;

void shadingfunc(int t) {
    if(ob > 0){
        for(auto& list: listocc){
            if(list.main == ob){

                coloring = true;
                list.Shades = shadingchoices.col(t);

                break;
            }
        }
    }
}

list<info> listinfo;
VertexBufferObject AnotherVBOobj;

void sceneFinaldrawn()
{
    if(First.size() != 0){
        GLenum mode = rendering == DiffShades::shadingwireframe ? GL_LINE_LOOP : GL_TRIANGLES;
       for (auto& list : listocc) {
           //set Stencil value
            glStencilFunc(GL_ALWAYS, list.main, -1);
            // in the vertex shader
            glUniformMatrix4fv(program.uniform("image"), 1, GL_FALSE, list.getterforImageOg().data());
            glUniformMatrix4fv(program.uniform("example"), 1, GL_FALSE, list.getterfororigstartChanged().data());
            if(ob == list.main && !coloring){
               glUniform3fv(program.uniform("shadething"), 1, shadingchoices.col(12).data());
            } else {
                glUniform3fv(program.uniform("shadething"), 1, list.Shades.data());
            }


            if(list.pic_ob.name == Thing::cube){
                glDrawElements(mode, list.pic_ob.limpoint, GL_UNSIGNED_INT, (unsigned int *) list.pic_ob.changelim);
            } else {
                glDrawElements(mode, list.pic_ob.limpoint, GL_UNSIGNED_INT, (unsigned int *) list.pic_ob.changelim);
            }
            glDrawElements(mode, list.pic_ob.limpoint, GL_UNSIGNED_INT, (void *) list.pic_ob.changelim);

            if(rendering == DiffShades::Shadeflat){
               glUniform3f(program.uniform("shadething"), 0.5, 0.5, 0.5);
               glDrawElements(GL_LINE_LOOP, list.pic_ob.limpoint, GL_UNSIGNED_INT, (void *) list.pic_ob.changelim);
           }
        }
    }
}


Eigen::MatrixXf norcen;
void bnfunction(info& pic_ob){

    norcen.conservativeResize(vet.rows(), vet.cols());
    unsigned int limpoint = pic_ob.limpoint;
    bary.conservativeResize(vet.rows(), vet.cols());
    Eigen::VectorXf ptex;
    unsigned int changelim = pic_ob.changelim;
    unsigned int change_v = pic_ob.change_v;
    ptex.conservativeResize(vet.cols());
    unsigned int pointchange = pic_ob.pointchange;


    for(int x = change_v; x < pointchange ; x++){
        bary.col(x) << 0.0, 0.0, 0.0;
        norcen.col(x) << 0.0, 0.0, 0.0;
        ptex(x) = 0;
    }

    for( int x=changelim; x < limpoint;)
    {

        Eigen::Vector3f V0 = vet.col(First[x]);
        Eigen::Vector3f V1 = vet.col(First[x+1]);
        Eigen::Vector3f V2 = vet.col(First[x+2]);

        Eigen::Vector3f normal = (V1-V0).cross(V2-V0).normalized();

        norcen.col(First[x]) += normal;
        ptex(First[x]) += 1;
        norcen.col(First[x+1]) += normal;
        ptex(First[x+1]) += 1;
        norcen.col(First[x+2]) += normal;
        ptex(First[x+2]) += 1;


        x = x +3;
    }

    for(int x = change_v; x < pointchange ; x++){
        norcen.col(x) = (norcen.col(x)/ptex(First[x])).normalized();
    }
    AnotherVBOobj.update(norcen);
}

void cursor_position_callback(GLFWwindow *window, double x, double y)
{
    if (trackingmouse)
    {

        int width, height;
        glfwGetWindowSize(window, &width, &height);


        Eigen::Vector4f p_screen(x,height-1-y,0,1);
        Eigen::Vector4f p_canonical((p_screen[0]/width)*2-1,(p_screen[1]/height)*2-1,0,1);

        for(auto& list: listocc){
            if(list.main == ob){
                Eigen::Vector4f p_world = scene && !matrixforScene.isZero() ? matrixforScene.inverse() * p_canonical : p_canonical;
                float translation_y  = p_world.y() - lookforb;

                float translation_x = p_world.x() - lookfora;

                Eigen::Matrix4f mat_v_new_one = moving(translation_x, translation_y, 0.0);
                list.imageOgchanged = mat_v_new_one  * list.imageOgchanged;
                list.origstartChanged = mat_v_new_one * list.origstartChanged;
                lookfora = p_world.x();
                lookforb = p_world.y();
            }
        }
    }
}


Eigen::Matrix4f getimagenewFunc(info pic_ob){
    float zoomingimagefact;
    Eigen::Matrix4f origstart;

    switch (pic_ob.name) {

        case Thing::cubewithbumps:
            zoomingimagefact = 0.07;
            break;

            case Thing::bunnyimage:
            zoomingimagefact = 2.5;
            break;
        case Thing::cube:
            zoomingimagefact = 0.2;
            break;


        default:
            break;
    }

    float rand_y = (rand() % 151)/100.0 -0.75; 

    float rand_x = (rand() % 151)/100.0 -0.75; 



    origstart = moving(rand_x, rand_y, 0) * moving(hitsimage - pic_ob.inside) * zooming(zoomingimagefact);
    return origstart;
}

Eigen::Vector3f moveCam(-1.0, 1.0, 2.0);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);


    int width, height;
    glfwGetWindowSize(window, &width, &height);


    Eigen::Vector4f p_screen(xpos,height-1-ypos,0,1);
    Eigen::Vector4f p_canonical((p_screen[0]/width)*2-1,(p_screen[1]/height)*2-1,0,1);
    Eigen::Vector4f p_world = scene && !matrixforScene.isZero() ? matrixforScene.inverse() * p_canonical : p_canonical;

    if(typeofchange == Do::Trans){
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            switch (action)
            {
                case GLFW_RELEASE:
                    switch (trackerformovement)
                    {
                        case 1:
                            trackingmouse = false;
                            break;
                        case 2:
                            ob = -1;
                            coloring = false;
                            trackerformovement = 0;

                        default:
                            break;
                    }
                    break;
                case GLFW_PRESS:
                    trackerformovement = trackerformovement + 1;
                    switch (trackerformovement)
                    {
                        case 1:
                        {
                            if(findposofmousewindow(window)){
                                trackingmouse = true;

                                lookforb = p_world.y();
                                lookfora = p_world.x();
                            }
                            break;
                        }
                        default:
                            break;
                    }

                default:
                    break;
            }
        }
    }
}


int ini_w = 640;

enum typeofproj{
    P,
    O
};

typeofproj choosePorO;
Eigen::Matrix4f camInfunc(Eigen::Matrix4f origstart){

    Eigen::Matrix4f typeofproj;
    Eigen::Matrix4f View;


    float sc_rat = 1.0f * ini_w / ini_length;

    Eigen::Matrix4f x;
    View = cammat(changeDirection, moveCam, hitsimage);

    if(choosePorO == typeofproj::O){
       typeofproj = viewO(0.0f, 2.0f, 0.0f, 2.0f*(1.0f/sc_rat), 0.1f, 10.0f) * moving(1.0, 1.0/sc_rat, 0.0);
    } else {
       typeofproj = modifiedperp(sc_rat, 10.f, 45.0, 0.1f);
    }
    x = typeofproj * View * origstart;
    return x;
}

void currentMod(){
    for(auto& list : listocc){
        list.imageOg =camInfunc(list.origstart);
    }
}


void window_size_callback(GLFWwindow* window, int width, int height)
{
    ini_w = width;
    ini_length = height;
    currentMod();
}

IndexBufferObject indo;

void changeCamfunc(Eigen::Vector3f camvmat){
    moveCam = moveCam + camvmat;
    if(First.size() > 0){
        for(auto& list: listocc){
            list.imageOg = camInfunc(list.origstart);
        }
    }
}

enum Oper
{
    Zooming,
    tilting
};

void putImage(Thing theimg){
    bool imgflag = false;
    for(auto const & pic_ob: listinfo){
        if(pic_ob.name == theimg){
            imgflag = true;
        }
    }
    if(!imgflag){
        unsigned int imguntil = First.size();
        unsigned int vuntil = vet.cols();
        string filename;
        switch (theimg) {

            case Thing::bunnyimage:
                filename = "bunny.off";
                break;
            case Thing::cubewithbumps:
                filename = "bumpy_cube.off";
                break;
            case Thing::cube:
                filename = "unit_cube.off";
                break;
            default:
                break;
        }
        Eigen::Vector3f objectCenter;
        if(loadMeshFromFile(filename, objectCenter)) {
            info newObject = info(listinfo.size()+1 , theimg, First.size() - imguntil, imguntil, vet.cols() - vuntil, vuntil, objectCenter);
            listinfo.push_back(newObject);
            bnfunction(newObject);

            indo.update(First);
            VBO.update(vet);


            if(initial_s){
                initial_s = false;
                program.bindVertexAttribArray("b", AnotherVBOobj);
                program.bindVertexAttribArray("a", VBO);

            }
        }
    }

    Eigen::Vector3f Shades;
    Eigen::Matrix4f mvp;
    switch (theimg) {
        case Thing::cube:
            Shades << shadingchoices.col(9);
            break;
        case Thing::bunnyimage:
            Shades << shadingchoices.col(11);
            break;
        case Thing::cubewithbumps:
            Shades << shadingchoices.col(10);
            break;


        default:
            break;
    }

    for(auto const& pic_ob: listinfo){
        if(pic_ob.name == theimg){
            Eigen::Matrix4f origstart = getimagenewFunc(pic_ob);
            Eigen::Matrix4f imageOg = camInfunc(origstart);
            occ list = occ(listocc.size()+1, pic_ob, imageOg, origstart, Shades);
            listocc.push_back(list);
        }
    }
}


void currentOperfunc(Oper transform, string action){
    if(ob > 0){
        for(auto& list: listocc){
            if(list.main == ob){
                Eigen::Matrix4f mat_v_new_one = Eigen::Matrix4f::Identity();
                switch (transform) {

                    case tilting:
                        if(action == "CounterClockWiseMovement"){
                            mat_v_new_one = moving(hitsimage - list.pic_ob.inside) * zaroundzoom(-10) * moving(list.pic_ob.inside - hitsimage);
                        }
                        else if(action == "MovementClockWise"){
                            mat_v_new_one =  moving(hitsimage - list.pic_ob.inside) * zaroundzoom(10) * moving(list.pic_ob.inside - hitsimage);
                        }
                        break;

                    case Zooming:
                        if(action == "DOWN"){
                            mat_v_new_one =  moving(hitsimage - list.pic_ob.inside) * zooming(0.75) * moving(list.pic_ob.inside - hitsimage);
                        }
                        else if(action == "UP"){
                            mat_v_new_one =  moving(hitsimage - list.pic_ob.inside) * zooming(1.25) * moving(list.pic_ob.inside - hitsimage);
                        }
                        break;

                    default:
                        break;
                }
                list.origstartChanged = mat_v_new_one * list.origstartChanged;
                list.imageOgchanged = mat_v_new_one * list.imageOgchanged;

                break;
            }
        }
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_RELEASE){
        // Update the position of the first vertex if the keys 1,2, or 3 are pressed
        switch (key)
        {

            case GLFW_KEY_O:
                choosePorO = typeofproj::O;
                currentMod();
                break;

            case GLFW_KEY_5:
                if(typeofchange == Do::Trans) {
                    shadingfunc(4);
                }
                break;

            case GLFW_KEY_DOWN:
                changeCamfunc(Eigen::Vector3f(0.0, -1.0, 0.0));
                break;

            case GLFW_KEY_9:
                if(typeofchange == Do::Trans) {
                    shadingfunc(8);
                }
                break;

            case GLFW_KEY_T:
                typeofchange = Do::Trans;
                ob = -1;
                coloring = false;
                break;

            case GLFW_KEY_2:
                if(typeofchange == Do::Inser){
                    putImage(Thing::cubewithbumps);
                } else if(typeofchange == Do::Trans) {
                     shadingfunc(1);
                }
                break;

            case GLFW_KEY_UP:
                changeCamfunc(Eigen::Vector3f(0.0, 1.0, 0.0));
                break;

            case GLFW_KEY_Q:
                if(typeofchange == Do::Trans) {
                    currentOperfunc(Oper::Zooming, "UP");
                }

            case GLFW_KEY_A:
                rendering = DiffShades::shadingwireframe;
                glUniform1i(program.uniform("wfpshades"), rendering);
                break;
            case GLFW_KEY_4:
                if(typeofchange == Do::Trans) {
                      shadingfunc(3);;
                }
                break;

            case GLFW_KEY_P:
                choosePorO = typeofproj::P;
                currentMod();
                break;
            case GLFW_KEY_LEFT:
                changeCamfunc(Eigen::Vector3f(-1.0, 0.0, 0.0));
                break;


            case GLFW_KEY_7:
                if(typeofchange == Do::Trans) {
                      shadingfunc(6);
                }
                break;
            case GLFW_KEY_S:
                rendering = DiffShades::Shadeflat;
                glUniform1i(program.uniform("wfpshades"), rendering);
                break;

            case GLFW_KEY_K:
                if(typeofchange == Do::Trans) {
                    currentOperfunc(Oper::tilting, "CounterClockWiseMovement");
                }
                break;
            case GLFW_KEY_3:
                if(typeofchange == Do::Inser){
                    putImage(Thing::bunnyimage);
                } else if(typeofchange == Do::Trans) {
                    shadingfunc(2);
                }
                break;

            case GLFW_KEY_D:
                rendering = DiffShades::shadingphong;
                glUniform1i(program.uniform("wfpshades"), rendering);
                break;

                break;
            case GLFW_KEY_6:
                if(typeofchange == Do::Trans) {
                    shadingfunc(5);
                }
                break;

            case GLFW_KEY_W:
                if(typeofchange == Do::Trans) {
                    currentOperfunc(Oper::Zooming, "DOWN");
                }
            case GLFW_KEY_L:
                if(typeofchange == Do::Trans) {
                    currentOperfunc(Oper::tilting, "MovementClockWise");
                }
                break;


            case GLFW_KEY_RIGHT:
                changeCamfunc(Eigen::Vector3f(1.0, 0.0, 0.0));
                break;

            case GLFW_KEY_1:
                if(typeofchange == Do::Inser){
                    putImage(Thing::cube);
                } else if(typeofchange == Do::Trans) {
                    shadingfunc(0);
                }
                break;

            case GLFW_KEY_8:
                if(typeofchange == Do::Trans) {
                    shadingfunc(7);
                }
                break;

            case GLFW_KEY_I:
                typeofchange = Do::Inser;
                break;
            default:
                break;
        }
    }
}

namespace LightSource {
    Eigen::Vector3f location = Eigen::Vector3f(0.0, 1.0, 2.0);
    Eigen::Vector3f Shades = Eigen::Vector3f(1.0, 1.0, 1.0);
};


int main(void)
{
    GLFWwindow *window;


    if (!glfwInit())
        return -1;


    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);


#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


    window = glfwCreateWindow(ini_w, ini_length, "Assignment 6", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }


    glfwMakeContextCurrent(window);

#ifndef __APPLE__
    glewExperimental = true;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char *)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));



    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();


    VBO.init();
    indo.init();
    AnotherVBOobj.init();
    
    shadingchoices.resize(3, 13);
    shadingchoices <<
    1.0, 0.32, 0.0, 0.0, 0.0, 0.4, 0.79, 0.25, 0.9, 0.3, 0.7, 0.8, 0.0,
    0.0, 0.54, 0.2, 1.0, 0.5, 0.0, 0.2, 1.0,  0.6,  0.2, 0.0, 0.3, 0.0,
    0.0, 0.7, 0.0, 1.0, 0.5, 0.0, 0.4,  0.75, 0.0, 0.3, 0.0, 0.4, 1.0;


    const GLchar *vertex_shader =
    "#version 150 core\n"
    "uniform vec3 placeofcam;"
    "uniform vec3 Shadelit;"
    "in vec3 a;"
    "uniform mat4 example;"
    "uniform vec3 shadething;"
    "in vec3 b;"
    "smooth out vec3 shadingforc;"
    "uniform mat4 image;"
    "uniform int wfpshades;"
    "flat out vec3 shadingforf;"
    "uniform vec3 plit;"

    "void main()"
    "{"
    "   float speckvalue = 0.5;"
    "   vec3 shadinspe = vec3(0.0, 0.0, 0.0);"
    "   gl_Position = image * vec4(a, 1.0);"
    "   vec3 xvt = mat3(transpose(inverse(example))) * b;"
    "   vec3 xvt_og = normalize(xvt);"
    "   float diffkvalue = 0.2;"
    "   vec3 dc = vec3(example * vec4(a, 1.0)); "
    "   float ambkvalue = 0.3;"
    "   vec3 shadingamb = ambkvalue * Shadelit;"
    "   vec3 wherelitcoming = normalize(plit - dc);"
    "   float diff = max(dot(xvt_og, wherelitcoming), 0.0);"
    "   vec3 shadindif = diffkvalue * diff * Shadelit;"

    "   if(wfpshades == 2){"
    "   vec3 viewDir = normalize(placeofcam - dc);"
    "   vec3 reflectDir = reflect(-wherelitcoming, xvt_og);"
    "   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);"
    "   shadinspe = speckvalue * spec * Shadelit; } "
    "   shadingforc = (shadingamb + shadindif + shadinspe) * shadething;"
    "   shadingforf = (shadingamb + shadindif + shadinspe) * shadething;"

    "}";
    const GLchar *fragment_shader =
    "#version 150 core\n"
    "uniform int wfpshades;"
    "flat in vec3 shadingforf;"
    "out vec4 finShade;"
    "smooth in vec3 shadingforc;"

    "void main()"
    "{"
    "   if(wfpshades == 1){"
    "       finShade = vec4(shadingforf, 1.0);"
    "   } else {"
    "       finShade = vec4(shadingforc, 1.0);"
    "   }"
  
    "}";


    program.init(vertex_shader, fragment_shader, "finShade");
    program.bind();


    glfwSetKeyCallback(window, key_callback);


    glfwSetMouseButtonCallback(window, mouse_button_callback);


    glfwSetCursorPosCallback(window, cursor_position_callback);
    

    glfwSetWindowSizeCallback(window, window_size_callback);
    
    glUniform3fv(program.uniform("plit"), 1, LightSource::location.data());
    glUniform3fv(program.uniform("Shadelit"), 1, LightSource::Shades.data());
    glUniform3fv(program.uniform("placeofcam"), 1, moveCam.data());
    glUniform1i(program.uniform("wfpshades"), DiffShades::shadingwireframe);


    while (!glfwWindowShouldClose(window))
    {

        VAO.bind();


        program.bind();


       glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

        glClearStencil(0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        
        // Enable blend //can take this off
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        

        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        
        sceneFinaldrawn();


        glfwSwapBuffers(window);


        glfwPollEvents();
    }


    program.free();
    VAO.free();
    VBO.free();
    indo.free();
    AnotherVBOobj.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
