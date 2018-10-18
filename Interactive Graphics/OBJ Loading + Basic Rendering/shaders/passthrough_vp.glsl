//	Passthrough for shader linking
//	-Robert Lach
//  --Adapted from NVidia Base Shader Template

varying vec3 normal;

void main()
{
    gl_Position = ftransform();
    normal = gl_NormalMatrix * gl_Normal;
}