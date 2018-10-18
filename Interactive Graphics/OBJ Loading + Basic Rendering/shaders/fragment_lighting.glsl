//	Phong Blinn Fragment Shader
//	-Robert Lach
//  --Adapted from NVidia Base Shader Template


uniform vec3 diffuseMaterial = vec3(0.0, 0.0, 1.0);
uniform vec3 specularMaterial = vec3(1.0, 1.0, 1.0);

uniform vec3 lightVec;

varying vec3 normal;

void main()
{
    vec3 normalVec = normalize(normal);
    vec3 eyeVec = vec3(0.0, 0.0, 1.0);
    vec3 halfVec = normalize(lightVec + eyeVec);
    vec3 diffuse = vec3(max(dot(normalVec, lightVec), 0.0)) * diffuseMaterial;
    vec3 specular = vec3(max(dot(normalVec, halfVec), 0.0));
    specular = pow(specular.x, 32.0) * specularMaterial;
    gl_FragColor.rgb = diffuse + specular;
    gl_FragColor.a = 1.0;
}
