// glsl implementation of refract function from Cg stdlib
vec3 refract(vec3 i, vec3 n, float eta)
{
    float cosi = dot(-i, n);
    float cost2 = 1.0 - eta * eta * (1.0 - cosi*cosi);
    vec3 t = eta*i + ((eta*cosi - sqrt(abs(cost2))) * n);
    return t * vec3(cost2 > 0.0);
}

// fresnel approximation
float fast_fresnel(vec3 I, vec3 N, vec3 fresnelValues)
{
    float power = fresnelValues.x;
    float scale = fresnelValues.y;
    float bias = fresnelValues.z;

    return bias + pow(1.0 - dot(I, N), power) * scale;
}

// inputs
varying vec3 normal;
varying vec3 incident;

// uniform inputs
uniform vec3 etaValues;
uniform vec3 fresnelValues;
uniform samplerCube environmentMap;

void main()
{
    // normalize incoming vectors
    vec3 normalVec = normalize(normal);
    vec3 incidentVec = normalize(incident);

    vec3 refractColor;

    // calculate refract color for each color channel
    refractColor.r = textureCube(environmentMap, refract(incidentVec, normalVec, etaValues.x)).r;
    refractColor.g = textureCube(environmentMap, refract(incidentVec, normalVec, etaValues.y)).g;
    refractColor.b = textureCube(environmentMap, refract(incidentVec, normalVec, etaValues.z)).b;

    // fetch reflection from environment map
    vec3 reflectColor = textureCube(environmentMap, reflect(incidentVec, normalVec)).rgb;

    vec3 fresnelTerm = vec3(fast_fresnel(-incidentVec, normalVec, fresnelValues));
    gl_FragColor = vec4(mix(refractColor, reflectColor, fresnelTerm), 1.0);
}
