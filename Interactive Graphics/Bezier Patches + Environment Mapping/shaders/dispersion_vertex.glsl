/******************************************************************************
File:  glsl_dispersion.glsl

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:

    Refractive dispersion (aka Chromatic Aberration)
    sgreen@nvidia.com 5/15/02

    GLSL implementation
    jallen@nvidia.com 12/10/03

******************************************************************************/

// outputs
varying vec3 normal;
varying vec3 incident;

// uniform inputs
uniform vec4 eyePos;
uniform vec4 displace;
uniform mat4 viewInverse;

void main()
{
    // deformation
    float deformation = sin(gl_Vertex.y * displace.x + displace.y) * displace.z;
    vec4 position = (gl_Vertex + vec4(gl_Normal, 1.0) * deformation);

    // transform normal to world space
    normal = gl_NormalMatrix * gl_Normal; // mul by modelIT

    // transform position to world space
    vec4 worldPos = gl_ModelViewMatrix * position; // mul by model

    // transform eye position to world space
    vec4 worldEyePos = viewInverse * eyePos; 

    // calculate incident vector
    incident = worldPos.xyz - worldEyePos.xyz;

    // output position
    gl_Position = gl_ModelViewProjectionMatrix * position;
}