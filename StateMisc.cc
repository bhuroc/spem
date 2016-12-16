/**************************************************
*
* $Id$
*
**************************************************/
/**
 *@file StateMisc.cc
 *@brief Not so crucial functions in State.cc.
 */

void start_stencil(void *, SoAction *)
{
    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glStencilFunc(GL_NEVER, 1, 1);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
}

void use_stencil(void *, SoAction *)
{
    glStencilFunc(GL_NOTEQUAL, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void stop_stencil(void*, SoAction *)
{
    glDisable(GL_STENCIL_TEST);
}


double screen_angle2length(double angle)
{
    // angle is in deg
    Vector<double> center=viewer.get_center_eye_in_screen();
    double center_eye_dist = norm(center);
    return center_eye_dist * angle * M_PI/180.; 
}

