#include "ShadowRenderer.h"

#include "raymath.h"
#include "rlgl.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr int kShadowTextureSlot = 10;

const char* kShadowVertexShader = R"GLSL(#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
void main() {
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
)GLSL";

const char* kShadowFragmentShader = R"GLSL(#version 330
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
out vec4 finalColor;
uniform vec3 lightDir;
uniform vec4 lightColor;
uniform vec4 ambient;
uniform vec3 viewPos;
uniform mat4 lightVP;
uniform sampler2D shadowMap;
uniform int shadowMapResolution;
void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 normal = normalize(fragNormal);
    vec3 l = normalize(-lightDir);
    vec3 viewD = normalize(viewPos - fragPosition);
    float nDotL = max(dot(normal, l), 0.0);
    vec3 diffuse = lightColor.rgb*nDotL;
    float specular = 0.0;
    if (nDotL > 0.0) specular = pow(max(0.0, dot(viewD, reflect(-l, normal))), 16.0);

    vec4 fragPosLightSpace = lightVP*vec4(fragPosition, 1.0);
    fragPosLightSpace.xyz /= max(fragPosLightSpace.w, 0.00001);
    fragPosLightSpace.xyz = fragPosLightSpace.xyz*0.5 + 0.5;
    vec3 shadowCoord = fragPosLightSpace.xyz;
    float shadow = 0.0;
    if (shadowCoord.x >= 0.0 && shadowCoord.x <= 1.0 &&
        shadowCoord.y >= 0.0 && shadowCoord.y <= 1.0 &&
        shadowCoord.z >= 0.0 && shadowCoord.z <= 1.0) {
        float bias = max(0.0002*(1.0 - dot(normal, l)), 0.00002);
        vec2 texel = vec2(1.0/float(shadowMapResolution));
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                float closestDepth = texture(shadowMap, shadowCoord.xy + vec2(x, y)*texel).r;
                shadow += (shadowCoord.z - bias > closestDepth) ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;
    }
    vec3 lit = texelColor.rgb*colDiffuse.rgb*(diffuse*(1.0 - shadow) + ambient.rgb);
    lit += vec3(specular*0.18)*(1.0 - shadow);
    finalColor = vec4(pow(max(lit, vec3(0.0)), vec3(1.0/2.2)), texelColor.a*colDiffuse.a);
}
)GLSL";
}

ShadowRenderer::~ShadowRenderer() {
    Shutdown();
}

bool ShadowRenderer::Init() {
    if (ready) return true;
    if (!IsWindowReady()) return false;

    shadowShader = LoadShaderFromMemory(kShadowVertexShader, kShadowFragmentShader);
    if (shadowShader.id == 0) return false;

    shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");
    lightDirectionLocation = GetShaderLocation(shadowShader, "lightDir");
    lightColorLocation = GetShaderLocation(shadowShader, "lightColor");
    ambientLocation = GetShaderLocation(shadowShader, "ambient");
    lightViewProjectionLocation = GetShaderLocation(shadowShader, "lightVP");
    shadowMapLocation = GetShaderLocation(shadowShader, "shadowMap");
    shadowMapResolutionLocation = GetShaderLocation(shadowShader, "shadowMapResolution");
    viewPositionLocation = GetShaderLocation(shadowShader, "viewPos");

    if (!CreateShadowMap(kShadowResolution, kShadowResolution)) {
        UnloadShader(shadowShader);
        shadowShader = {};
        return false;
    }

    cubeModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    if (cubeModel.meshCount == 0 || cubeModel.materialCount == 0) {
        DestroyShadowMap();
        UnloadShader(shadowShader);
        shadowShader = {};
        return false;
    }
    cubeModel.materials[0].shader = shadowShader;

    const Vector4 lightColor = { 1.0f, 0.98f, 0.94f, 1.0f };
    const Vector4 ambient = { 0.24f, 0.26f, 0.30f, 1.0f };
    const int resolution = kShadowResolution;
    SetShaderValue(shadowShader, lightColorLocation, &lightColor, SHADER_UNIFORM_VEC4);
    SetShaderValue(shadowShader, ambientLocation, &ambient, SHADER_UNIFORM_VEC4);
    SetShaderValue(shadowShader, shadowMapResolutionLocation, &resolution, SHADER_UNIFORM_INT);
    SetShaderValue(shadowShader, lightDirectionLocation, &lightDirection, SHADER_UNIFORM_VEC3);
    ready = true;
    return true;
}

void ShadowRenderer::Shutdown() {
    if (cubeModel.meshCount > 0) UnloadModel(cubeModel);
    cubeModel = {};
    DestroyShadowMap();
    if (shadowShader.id > 0) UnloadShader(shadowShader);
    shadowShader = {};
    ready = false;
}

bool ShadowRenderer::CreateShadowMap(int width, int height) {
    shadowMap = {};
    shadowMap.id = rlLoadFramebuffer();
    shadowMap.texture.width = width;
    shadowMap.texture.height = height;
    if (shadowMap.id <= 0) return false;

    rlEnableFramebuffer(shadowMap.id);
    shadowMap.depth.id = rlLoadTextureDepth(width, height, false);
    shadowMap.depth.width = width;
    shadowMap.depth.height = height;
    shadowMap.depth.format = 19; // RL_PIXELFORMAT_DEPTH_24BIT
    shadowMap.depth.mipmaps = 1;
    rlFramebufferAttach(shadowMap.id, shadowMap.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
    const bool complete = rlFramebufferComplete(shadowMap.id);
    rlDisableFramebuffer();
    if (!complete) {
        DestroyShadowMap();
        return false;
    }
    return true;
}

void ShadowRenderer::DestroyShadowMap() {
    if (shadowMap.id > 0) rlUnloadFramebuffer(shadowMap.id);
    shadowMap = {};
}

void ShadowRenderer::AdjustLight(Vector2 delta) {
    lightDirection.x = std::clamp(lightDirection.x + delta.x, -0.85f, 0.85f);
    lightDirection.z = std::clamp(lightDirection.z + delta.y, -0.85f, 0.85f);
    lightDirection.y = -1.0f;
    lightDirection = Vector3Normalize(lightDirection);
    if (ready) SetShaderValue(shadowShader, lightDirectionLocation, &lightDirection, SHADER_UNIFORM_VEC3);
}

void ShadowRenderer::RenderShadowMap(const std::vector<Entity>& entities) {
    if (!ready) return;

    Vector3 center = { 0.0f, 0.0f, 0.0f };
    float extent = 8.0f;
    if (!entities.empty()) {
        Vector3 minBounds = entities.front().GetBoundingBox().min;
        Vector3 maxBounds = entities.front().GetBoundingBox().max;
        for (const Entity& entity : entities) {
            const BoundingBox bounds = entity.GetBoundingBox();
            minBounds.x = std::min(minBounds.x, bounds.min.x);
            minBounds.y = std::min(minBounds.y, bounds.min.y);
            minBounds.z = std::min(minBounds.z, bounds.min.z);
            maxBounds.x = std::max(maxBounds.x, bounds.max.x);
            maxBounds.y = std::max(maxBounds.y, bounds.max.y);
            maxBounds.z = std::max(maxBounds.z, bounds.max.z);
        }
        center = Vector3Scale(Vector3Add(minBounds, maxBounds), 0.5f);
        const Vector3 span = Vector3Subtract(maxBounds, minBounds);
        extent = std::max(8.0f, std::max(span.x, std::max(span.y, span.z))*0.75f + 4.0f);
    }

    lightCamera = {};
    lightCamera.position = Vector3Subtract(center, Vector3Scale(lightDirection, extent*2.0f));
    lightCamera.target = center;
    lightCamera.up = { 0.0f, 1.0f, 0.0f };
    lightCamera.fovy = extent*2.0f;
    lightCamera.projection = CAMERA_ORTHOGRAPHIC;

    rlEnableFramebuffer(shadowMap.id);
    rlViewport(0, 0, kShadowResolution, kShadowResolution);
    rlClearScreenBuffers();
    BeginMode3D(lightCamera);
        lightView = rlGetMatrixModelview();
        lightProjection = rlGetMatrixProjection();
        for (const Entity& entity : entities) {
            const Vector3 scale = {
                std::max(0.001f, std::fabs(entity.boyut.x)),
                std::max(0.001f, std::fabs(entity.boyut.y)),
                std::max(0.001f, std::fabs(entity.boyut.z))
            };
            DrawModelEx(cubeModel, entity.pozisyon, { 0.0f, 1.0f, 0.0f }, 0.0f, scale, WHITE);
        }
    EndMode3D();
    rlDisableFramebuffer();
    rlViewport(0, 0, GetRenderWidth(), GetRenderHeight());
    lightViewProjection = MatrixMultiply(lightView, lightProjection);
}

void ShadowRenderer::BeginMainPass(const Camera3D& camera) {
    if (!ready) return;
    SetShaderValue(shadowShader, viewPositionLocation, &camera.position, SHADER_UNIFORM_VEC3);
    SetShaderValueMatrix(shadowShader, lightViewProjectionLocation, lightViewProjection);
    rlEnableShader(shadowShader.id);
    rlActiveTextureSlot(kShadowTextureSlot);
    rlEnableTexture(shadowMap.depth.id);
    rlSetUniform(shadowMapLocation, &kShadowTextureSlot, SHADER_UNIFORM_INT, 1);
}

void ShadowRenderer::DrawEntity(const Entity& entity) const {
    if (!ready) {
        DrawCube(entity.pozisyon, std::fabs(entity.boyut.x), std::fabs(entity.boyut.y),
                 std::fabs(entity.boyut.z), entity.renk);
        return;
    }
    const Vector3 scale = {
        std::max(0.001f, std::fabs(entity.boyut.x)),
        std::max(0.001f, std::fabs(entity.boyut.y)),
        std::max(0.001f, std::fabs(entity.boyut.z))
    };
    DrawModelEx(cubeModel, entity.pozisyon, { 0.0f, 1.0f, 0.0f }, 0.0f, scale, entity.renk);
}

void ShadowRenderer::EndMainPass() {
    if (!ready) return;
    rlActiveTextureSlot(kShadowTextureSlot);
    rlDisableTexture();
    rlDisableShader();
}
