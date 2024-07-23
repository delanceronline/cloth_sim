// Matrix.h: interface for the Matrix class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MATRIX_H__04336E76_9A13_413C_8A7C_E4CCAB3252EB__INCLUDED_)
#define AFX_MATRIX_H__04336E76_9A13_413C_8A7C_E4CCAB3252EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Math.h>
#include <windows.h>

class Matrix  
{
public:
	bool ToDiagonal(void);
	bool Subtract(Matrix *pMat);
	bool Transpose(void);
	bool Add(Matrix *pMat);
	//Pointer to array of matrix.
	float *m;
	//Numbers of rows and columns of matrix.
	int numRows, numColumns;						

	//Member function to exchange two rows of matrix.
	void ExchangeRow(int Row1, int Row2);
	//Member function to reset dimension of matrix.
	void ResetDimension(int Row, int Column);
	//Member function to set matrix to identity.
	void SetToIdentity(void);
	//Member function to calculate inverse of matrix.
	Matrix * Inverse(void);
	//Operator to proceed product of two matrices.
	Matrix * operator*(const Matrix& mat_in);
	//Member function to proceed product of two matrices.
	bool Product(const Matrix *mat_in, Matrix *result);
	//Member function to retrieve value of matrix.
	float GetVal(int Row, int Column);				
	//Member function to set value of matrix.
	bool SetVal(int Row, int Column, float val);
	//Member function to time a scalar value to every element in this matrix.
	bool ScalarProduct(float var);
	//Operator for assigning between matrices.
	Matrix& operator=(const Matrix& mat_in);
	//Member function for assigning between matrices.
	bool EqualTo(const Matrix *mat_in);	

	Matrix();
	//Constructor to define matrix with Row x Column dimension.
	Matrix(int Row, int Column);					
	//Destroyer to release resource of this class.
	virtual ~Matrix();								

private:
	//Member function to determine whether two floats are equal with precision up to 1e-5.
	bool IsEqual(float var1, float var2);			
};

#endif // !defined(AFX_MATRIX_H__04336E76_9A13_413C_8A7C_E4CCAB3252EB__INCLUDED_)
