//	Phong Blinn Vertex Shader
//	-Robert Lach
//  --Adapted from NVidia Base Shader Template

uniform vec3 diffuseMaterial = vec3(0.0, 0.0, 1.0);
uniform vec3 specularMaterial = vec3(1.0, 1.0, 1.0);

uniform vec3 lightVec;

void main()
{
    gl_Position = ftransform();
    vec3 normalVec = normalize(gl_NormalMatrix * gl_Normal);
    vec3 eyeVec = vec3(0.0, 0.0, 1.0);
    vec3 halfVec = normalize(lightVec + eyeVec);
    vec3 diffuse = vec3(max(dot(normalVec, lightVec), 0.0)) * diffuseMaterial;
    vec3 specular = vec3(max(dot(normalVec, halfVec), 0.0));
    specular = pow(specular.x, 32.0) * specularMaterial;
    gl_FrontColor.rgb = diffuse + specular;
    gl_FrontColor.a = 1.0;
}
