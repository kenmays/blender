//
//   Copyright 2013 Pixar
//
//   Licensed under the Apache License, Version 2.0 (the "Apache License")
//   with the following modification; you may not use this file except in
//   compliance with the Apache License and the following modification to it:
//   Section 6. Trademarks. is deleted and replaced with:
//
//   6. Trademarks. This License does not grant permission to use the trade
//      names, trademarks, service marks, or product names of the Licensor
//      and its affiliates, except as required to comply with Section 4(c) of
//      the License and to reproduce the content of the NOTICE file.
//
//   You may obtain a copy of the Apache License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the Apache License with the above modification is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
//   KIND, either express or implied. See the Apache License for the specific
//   language governing permissions and limitations under the Apache License.
//

//------------------------------------------------------------------------------

OsdPatchCoord GetPatchCoord(int coordIndex)
{
  return patchCoords[coordIndex];
}

OsdPatchArray GetPatchArray(int arrayIndex)
{
  return patchArrayBuffer[arrayIndex];
}

OsdPatchParam GetPatchParam(int patchIndex)
{
  return patchParamBuffer[patchIndex];
}

//------------------------------------------------------------------------------

struct Vertex {
  float vertexData[LENGTH];
};

void clear(out Vertex v)
{
  for (int i = 0; i < LENGTH; ++i) {
    v.vertexData[i] = 0;
  }
}

Vertex readVertex(int index)
{
  Vertex v;
  int vertexIndex = srcOffset + index * SRC_STRIDE;
  for (int i = 0; i < LENGTH; ++i) {
    v.vertexData[i] = srcVertexBuffer[vertexIndex + i];
  }
  return v;
}

void writeVertex(int index, Vertex v)
{
  int vertexIndex = dstOffset + index * DST_STRIDE;
  for (int i = 0; i < LENGTH; ++i) {
    dstVertexBuffer[vertexIndex + i] = v.vertexData[i];
  }
}

void addWithWeight(inout Vertex v, const Vertex src, float weight)
{
  for (int i = 0; i < LENGTH; ++i) {
    v.vertexData[i] += weight * src.vertexData[i];
  }
}

#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_1ST_DERIVATIVES)
void writeDu(int index, Vertex du)
{
  int duIndex = duDesc.x + index * duDesc.z;
  for (int i = 0; i < LENGTH; ++i) {
    duBuffer[duIndex + i] = du.vertexData[i];
  }
}

void writeDv(int index, Vertex dv)
{
  int dvIndex = dvDesc.x + index * dvDesc.z;
  for (int i = 0; i < LENGTH; ++i) {
    dvBuffer[dvIndex + i] = dv.vertexData[i];
  }
}
#endif

#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_2ND_DERIVATIVES)
void writeDuu(int index, Vertex duu)
{
  int duuIndex = duuDesc.x + index * duuDesc.z;
  for (int i = 0; i < LENGTH; ++i) {
    duuBuffer[duuIndex + i] = duu.vertexData[i];
  }
}

void writeDuv(int index, Vertex duv)
{
  int duvIndex = duvDesc.x + index * duvDesc.z;
  for (int i = 0; i < LENGTH; ++i) {
    duvBuffer[duvIndex + i] = duv.vertexData[i];
  }
}

void writeDvv(int index, Vertex dvv)
{
  int dvvIndex = dvvDesc.x + index * dvvDesc.z;
  for (int i = 0; i < LENGTH; ++i) {
    dvvBuffer[dvvIndex + i] = dvv.vertexData[i];
  }
}
#endif

//------------------------------------------------------------------------------
// PERFORMANCE: stride could be constant, but not as significant as length

void main()
{

  int current = int(gl_GlobalInvocationID.x);

  OsdPatchCoord coord = GetPatchCoord(current);
  OsdPatchArray array = GetPatchArray(coord.arrayIndex);
  OsdPatchParam param = GetPatchParam(coord.patchIndex);

  int patchType = OsdPatchParamIsRegular(param) ? array.regDesc : array.desc;

  float wP[20], wDu[20], wDv[20], wDuu[20], wDuv[20], wDvv[20];
  int nPoints = OsdEvaluatePatchBasis(
      patchType, param, coord.s, coord.t, wP, wDu, wDv, wDuu, wDuv, wDvv);

  Vertex dst, du, dv, duu, duv, dvv;
  clear(dst);
  clear(du);
  clear(dv);
  clear(duu);
  clear(duv);
  clear(dvv);

  int indexBase = array.indexBase + array.stride * (coord.patchIndex - array.primitiveIdBase);

  for (int cv = 0; cv < nPoints; ++cv) {
    int index = patchIndexBuffer[indexBase + cv];
    addWithWeight(dst, readVertex(index), wP[cv]);
    addWithWeight(du, readVertex(index), wDu[cv]);
    addWithWeight(dv, readVertex(index), wDv[cv]);
    addWithWeight(duu, readVertex(index), wDuu[cv]);
    addWithWeight(duv, readVertex(index), wDuv[cv]);
    addWithWeight(dvv, readVertex(index), wDvv[cv]);
  }
  writeVertex(current, dst);

#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_1ST_DERIVATIVES)
  if (duDesc.y > 0) {  // length
    writeDu(current, du);
  }
  if (dvDesc.y > 0) {
    writeDv(current, dv);
  }
#endif
#if defined(OPENSUBDIV_GLSL_COMPUTE_USE_2ND_DERIVATIVES)
  if (duuDesc.y > 0) {  // length
    writeDuu(current, duu);
  }
  if (duvDesc.y > 0) {  // length
    writeDuv(current, duv);
  }
  if (dvvDesc.y > 0) {
    writeDvv(current, dvv);
  }
#endif
}
