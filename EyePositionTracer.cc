/**
 *@file EyePositionTracer.cc
 *@brief 
 */
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoRotationXYZ.h>
#include <VML/Graphics/inventor.h>
#include "EyePositionTracer.h"

EyePositionTracer::EyePositionTracer()
{
    // Build the scene
    root = new SoSeparator;
    root->ref();

    SoMaterial *red = new SoMaterial;
    red->diffuseColor.setValue(1,0,0);
    SoMaterial *green = new SoMaterial;
    green->diffuseColor.setValue(0,1,0);

    SoCone *pointer = new SoCone;
    pointer->bottomRadius = 5;
    pointer->height = 10;

    SoSphere *eye = new SoSphere;

    // Left eye and right eye
    show_eye = new SoSwitch;  root->addChild(show_eye);
    showing_eye_overlay = false;
    show_eye->whichChild = SO_SWITCH_NONE;
    ADD_NODE0(show_eye, leye, SoSeparator);
    left_p = new SoTranslation;
    leye->addChild(left_p);
    leye->addChild(red);
    leye->addChild(eye);
    ADD_NODE0(show_eye, reye, SoSeparator);
    right_p = new SoTranslation;
    reye->addChild(right_p);
    reye->addChild(green);
    reye->addChild(eye);
    
    // Left tracer and right
    // Left tracer resides on the left and top
    // Right tracer is on the right and bottom
    ADD_NODE0(root,ltracer,SoSeparator);
    ADD_NODE0(ltracer,ltx,SoSeparator);
    ADD_NODE0(ltracer,lty,SoSeparator);
    left_x = new SoTranslation;
    ltx->addChild(red);
    ltx->addChild(left_x);
    ADD_NODE2(ltx,turn2right,SoRotationXYZ,axis,SoRotationXYZ::Z,angle,-M_PI/2);
    ltx->addChild(pointer);

    left_y = new SoTranslation;
    lty->addChild(red);
    lty->addChild(left_y);
    ADD_NODE2(lty,turn2down,SoRotationXYZ,axis,SoRotationXYZ::Z,angle,-M_PI);
    lty->addChild(pointer);

    ADD_NODE0(root,rtracer,SoSeparator);
    ADD_NODE0(rtracer,rtx,SoSeparator);
    ADD_NODE0(rtracer,rty,SoSeparator);
    right_x = new SoTranslation;
    rtx->addChild(green);
    ADD_NODE2(rtx,turn2left,SoRotationXYZ,axis,SoRotationXYZ::Z,angle,M_PI/2);
    rtx->addChild(pointer);

    right_y = new SoTranslation;
    rty->addChild(green);
    rty->addChild(left_y);
    rty->addChild(pointer);

    // Default screen
    x_scale = 1280*0.3/640;
    y_scale = 960*0.3/480;
    x_center = 640/2;
    y_center = 320/2;

    x_edge = 1280*0.3/2;
    y_edge = 960*0.3/2;

    margin = 5;//5mm
}

void EyePositionTracer::set_screen_info(double sw, double sh, double cw, double ch, int pw, int ph)
{
    x_scale = cw/pw;
    y_scale = ch/ph;
    x_center = pw/2;
    y_center = ph/2;

    x_edge = sw/2-margin;
    y_edge = sh/2-margin;
}

void EyePositionTracer::update_gaze(double lx, double ly, double rx, double ry)
{
    // lx, ly, rx and ry are raw pixel values
    // Transform then to screen 
    lx = x_scale * (lx - x_center);
    ly = y_scale * (y_center - ly);

    left_x->translation.setValue(lx, y_edge, 0);
    left_y->translation.setValue(-x_edge, ly, 0);
    left_p->translation.setValue(lx, ly, 0);

    rx = x_scale * (rx - x_center);
    ry = y_scale * (y_center - ry);

    right_x->translation.setValue(rx, -y_edge, 0);
    right_y->translation.setValue(x_edge, ry, 0);
    right_p->translation.setValue(rx, ry, 0);
}


