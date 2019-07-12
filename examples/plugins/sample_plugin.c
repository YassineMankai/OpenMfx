/*
 * Copyright 2019 Elie Michel
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "util/ofx_util.h"

#include "ofxCore.h"
#include "ofxMeshEffect.h"

#include <stdio.h>
#include <string.h>

#define kMainInput "MainInput"
#define kMainOutput "MainOutput"

typedef struct PluginRuntime {
    OfxHost *host;
    OfxPropertySuiteV1 *propertySuite;
    OfxMeshEffectSuiteV1 *meshEffectSuite;
} PluginRuntime;

PluginRuntime plugin0_runtime;
PluginRuntime plugin1_runtime;

static OfxStatus plugin0_load(PluginRuntime *runtime) {
    OfxHost *h = runtime->host;
    runtime->propertySuite = (OfxPropertySuiteV1*)h->fetchSuite(h->host, kOfxPropertySuite, 1);
    runtime->meshEffectSuite = (OfxMeshEffectSuiteV1*)h->fetchSuite(h->host, kOfxMeshEffectSuite, 1);
    return kOfxStatOK;
}

static OfxStatus plugin0_describe(const PluginRuntime *runtime, OfxMeshEffectHandle meshEffect) {
    if (NULL == runtime->propertySuite || NULL == runtime->meshEffectSuite) {
        return kOfxStatErrMissingHostFeature;
    }

    OfxStatus status;
    OfxPropertySetHandle propHandle;

    status = runtime->meshEffectSuite->getPropertySet(meshEffect, &propHandle);
    printf("Suite method 'getPropertySet' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->propertySuite->propSetString(propHandle, kOfxMeshEffectPropContext, 0, kOfxMeshEffectContextFilter);
    printf("Suite method 'propSetString' returned status %d (%s)\n", status, getOfxStateName(status));

    // Shall move into "describe in context" when it will exist
    OfxPropertySetHandle inputProperties;
    status = runtime->meshEffectSuite->inputDefine(meshEffect, kMainInput, &inputProperties);
    printf("Suite method 'inputDefine' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input");
    printf("Suite method 'propSetString' returned status %d (%s)\n", status, getOfxStateName(status));

    OfxPropertySetHandle outputProperties;
    status = runtime->meshEffectSuite->inputDefine(meshEffect, kMainOutput, &outputProperties); // yes, output are also "inputs", I should change this name in the API
    printf("Suite method 'inputDefine' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->propertySuite->propSetString(outputProperties, kOfxPropLabel, 0, "Main Output");
    printf("Suite method 'propSetString' returned status %d (%s)\n", status, getOfxStateName(status));

    return kOfxStatOK;
}

static OfxStatus plugin0_cook(PluginRuntime *runtime, OfxMeshEffectHandle meshEffect) {
    OfxStatus status;
    OfxMeshInputHandle input, output;
    OfxPropertySetHandle propertySet;

    status = runtime->meshEffectSuite->inputGetHandle(meshEffect, kMainInput, &input, &propertySet);
    printf("Suite method 'inputGetHandle' returned status %d (%s)\n", status, getOfxStateName(status));
    if (status != kOfxStatOK) {
        return kOfxStatErrUnknown;
    }

    status = runtime->meshEffectSuite->inputGetHandle(meshEffect, kMainOutput, &output, &propertySet);
    printf("Suite method 'inputGetHandle' returned status %d (%s)\n", status, getOfxStateName(status));
    if (status != kOfxStatOK) {
        return kOfxStatErrUnknown;
    }

    OfxTime time = 0;
    OfxPropertySetHandle input_mesh;
    status = runtime->meshEffectSuite->inputGetMesh(input, time, &input_mesh);
    printf("Suite method 'inputGetMesh' returned status %d (%s)\n", status, getOfxStateName(status));

    int input_point_count;
    status = runtime->propertySuite->propGetInt(input_mesh, kOfxMeshPropPointCount, 0, &input_point_count);
    printf("Suite method 'propGetInt' returned status %d (%s)\n", status, getOfxStateName(status));

    float *input_points;
    status = runtime->propertySuite->propGetPointer(input_mesh, kOfxMeshPropPointData, 0, (void**)&input_points);
    printf("Suite method 'propGetPointer' returned status %d (%s)\n", status, getOfxStateName(status));

    printf("DEBUG: Found %d in input mesh\n", input_point_count);

    // TODO: store input data

    status = runtime->meshEffectSuite->inputReleaseMesh(input_mesh);
    printf("Suite method 'inputReleaseMesh' returned status %d (%s)\n", status, getOfxStateName(status));

    // TODO: core cook

    OfxPropertySetHandle output_mesh;
    status = runtime->meshEffectSuite->inputGetMesh(output, time, &output_mesh);
    printf("Suite method 'inputGetMesh' returned status %d (%s)\n", status, getOfxStateName(status));

    int output_point_count = 0, output_vertex_count = 0, output_face_count = 0;

    // TODO: Consolidate geo counts

    printf("DEBUG: Allocating output mesh data: %d points, %d vertices, %d faces\n", output_point_count, output_vertex_count, output_face_count);

    status = runtime->meshEffectSuite->meshAlloc(output_mesh, output_point_count, output_vertex_count, output_face_count);
    printf("Suite method 'meshAlloc' returned status %d (%s)\n", status, getOfxStateName(status));

    float *output_points;
    status = runtime->propertySuite->propGetPointer(input_mesh, kOfxMeshPropPointData, 0, (void**)&output_points);
    printf("Suite method 'propGetPointer' returned status %d (%s)\n", status, getOfxStateName(status));

    int *output_vertices;
    status = runtime->propertySuite->propGetPointer(input_mesh, kOfxMeshPropVertexData, 0, (void**)&output_vertices);
    printf("Suite method 'propGetPointer' returned status %d (%s)\n", status, getOfxStateName(status));

    int *output_faces;
    status = runtime->propertySuite->propGetPointer(input_mesh, kOfxMeshPropFaceData, 0, (void**)&output_faces);
    printf("Suite method 'propGetPointer' returned status %d (%s)\n", status, getOfxStateName(status));

    // TODO: Fill data

    status = runtime->meshEffectSuite->inputReleaseMesh(output_mesh);
    printf("Suite method 'inputReleaseMesh' returned status %d (%s)\n", status, getOfxStateName(status));

    return kOfxStatOK;
}

static OfxStatus plugin0_mainEntry(const char *action,
                                   const void *handle,
                                   OfxPropertySetHandle inArgs,
                                   OfxPropertySetHandle outArgs) {
    if (0 == strcmp(action, kOfxActionLoad)) {
        return plugin0_load(&plugin0_runtime);
    }
    if (0 == strcmp(action, kOfxActionDescribe)) {
        return plugin0_describe(&plugin0_runtime, (OfxMeshEffectHandle)handle);
    }
    if (0 == strcmp(action, kOfxActionCreateInstance)) {
        return kOfxStatOK;
    }
    if (0 == strcmp(action, kOfxActionDestroyInstance)) {
        return kOfxStatOK;
    }
    if (0 == strcmp(action, kOfxMeshEffectActionCook)) {
        return plugin0_cook(&plugin0_runtime, (OfxMeshEffectHandle)handle);
    }
    return kOfxStatReplyDefault;
}

static void plugin0_setHost(OfxHost *host) {
    plugin0_runtime.host = host;
}

static OfxStatus plugin1_mainEntry(const char *action,
                                   const void *handle,
                                   OfxPropertySetHandle inArgs,
                                   OfxPropertySetHandle outArgs) {
    (void)action;
    (void)handle;
    (void)inArgs;
    (void)outArgs;
    return kOfxStatReplyDefault;
}

static void plugin1_setHost(OfxHost *host) {
    plugin1_runtime.host = host;
}

OfxExport int OfxGetNumberOfPlugins(void) {
    return 2;
}

OfxExport OfxPlugin *OfxGetPlugin(int nth) {
    static OfxPlugin plugins[] = {
        {
        /* pluginApi */          kOfxMeshEffectPluginApi,
        /* apiVersion */         kOfxMeshEffectPluginApiVersion,
        /* pluginIdentifier */   "MfxSamplePlugin0",
        /* pluginVersionMajor */ 1,
        /* pluginVersionMinor */ 0,
        /* setHost */            plugin0_setHost,
        /* mainEntry */          plugin0_mainEntry
        },
        {
        /* pluginApi */          kOfxMeshEffectPluginApi,
        /* apiVersion */         kOfxMeshEffectPluginApiVersion,
        /* pluginIdentifier */   "MfxSamplePlugin1",
        /* pluginVersionMajor */ 1,
        /* pluginVersionMinor */ 0,
        /* setHost */            plugin1_setHost,
        /* mainEntry */          plugin1_mainEntry
        }
    };
    return plugins + nth;
}
