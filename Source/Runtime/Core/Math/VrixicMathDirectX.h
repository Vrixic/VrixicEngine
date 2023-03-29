/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <DirectXMath.h>

/* A float4 vector where the X component of the vector is stored in the lowest 32 bits */
typedef DirectX::XMVECTOR VectorRegister;

/* returns and makes a vector with 4 floats */
inline VectorRegister MakeVectorRegister(float x, float y, float z, float w)
{
	return DirectX::XMVectorSet(x, y, z, w);
}

/* returns and makes a vector with 4 floats */
inline VectorRegister MakeVectorRegister(const float* v)
{
	return DirectX::XMVectorSet(v[0], v[1], v[2], v[3]);
}

/* stores a vector register into a Vector4D */
inline void StoreVectorRegister(float* v, VectorRegister& vectorRegister)
{
	DirectX::XMFLOAT4 v1;
	DirectX::XMStoreFloat4(&v1, vectorRegister);
	v[0] = v1.x;
	v[1] = v1.y;
	v[2] = v1.z;
	v[3] = v1.w;
}

/* Multiplies two matrices and result is returned via Param1*/
inline void VectorRegisterMatrixMultiply(float* result, const float* matrix1, const float* matrix2)
{
	DirectX::XMMATRIX xMat1 = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)(matrix1));
	DirectX::XMMATRIX xMat2 = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)(matrix2));
	DirectX::XMMATRIX xResult = DirectX::XMMatrixMultiply(xMat1, xMat2);
	DirectX::XMStoreFloat4x4A((DirectX::XMFLOAT4X4A*)(result), xResult);
}

/* A Homogenous transform */
inline VectorRegister TransformVectorByMatrix(const VectorRegister& V1, const float* Transform)
{
	DirectX::XMMATRIX matrix = DirectX::XMLoadFloat4x4A((const DirectX::XMFLOAT4X4A*)(Transform));
	return DirectX::XMVector4Transform(V1, matrix);
}