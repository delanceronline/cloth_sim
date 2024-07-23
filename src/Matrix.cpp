// Matrix.cpp: implementation of the Matrix class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Matrix.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Matrix::Matrix()
{
	m = NULL;
	numRows = 0;
	numColumns = 0;
}

Matrix::Matrix(int Row, int Column)
{
	m = new float[Row * Column];
	numRows = Row;
	numColumns = Column;

	for(int i = 0; i < Row * Column; i++)
		m[i] = 0.0f;
}

float Matrix::GetVal(int Row, int Column)
{
	return m[(numColumns) * Row + Column];
}

bool Matrix::SetVal(int Row, int Column, float val)
{
	m[(numColumns) * Row + Column] = val;

	return true;
}

Matrix::~Matrix()
{
	if(m != NULL)
	{
		delete[] m;
		m = NULL;
	}
}

Matrix* Matrix::operator *(const Matrix &mat_in)
{
	Matrix* result = NULL;

	if(numColumns == mat_in.numRows)
	{
		int i, j, k, var;
		float temp = 0.0f;

		//result.ResetDimension(numRows, mat_in.numColumns);
		result = new Matrix(numRows, mat_in.numColumns);

		for(k = 0; k < numRows; k++)
		{
			var = (mat_in.numColumns) * k;
			for(i = 0; i < mat_in.numColumns; i++)
			{
				for(j = 0; j < numColumns; j++)
					temp += m[(numColumns) * k + j] * mat_in.m[(mat_in.numColumns) * j + i];

				result->m[var + i] = temp;

				temp = 0.0f;
			}
		}
	}

	return result;
}

Matrix& Matrix::operator =(const Matrix& mat_in)
{
	if(&mat_in == this)
		return *this;

	ResetDimension(mat_in.numRows, mat_in.numColumns);
	for(int i = 0; i < numRows * numColumns; i++)
		this->m[i] = mat_in.m[i];

	return *this;
}

Matrix *Matrix::Inverse()
{
	Matrix *Left_Mat = NULL;
	Matrix *Right_Mat = NULL;

	if(numRows == numColumns)
	{
		int i, j, k;
		float var;

		//Left_Mat will be left matrix of augmented matrix.
		Left_Mat = new Matrix(numRows, numColumns);
		
		//Right_Mat will be initially right identity matrix of augmented matrix.
		Right_Mat = new Matrix(numRows, numColumns);
		Right_Mat->SetToIdentity();
		
		//Copy matrix values to Left_Mat->m.
		for(i = 0; i < numRows * numColumns; i++)
			Left_Mat->m[i] = m[i];

		//Loop through each row to make diagonal value to 1 and eliminate other rows above or below.
		for(i = 0; i < numRows; i++)
		{
			//if(Left_Mat->GetVal(i, i) == 0.0f)
			if(IsEqual(Left_Mat->GetVal(i, i), 0.0f))
			{
				//Exchange rows.
				for(j = i + 1; j < numRows; j++)
				{
					if(!IsEqual(Left_Mat->GetVal(j, i), 0.0f))
						ExchangeRow(i, j);
				}

				if(IsEqual(Left_Mat->GetVal(i, i), 0.0f))
				{
					delete Left_Mat;
					delete Right_Mat;

					return NULL;
				}

			}

				//If diagonal value is not 1, make it to 1 by division.
				if(!IsEqual(Left_Mat->GetVal(i, i), 1.0f))
				{
					var = Left_Mat->GetVal(i, i);
					//Make this diagonal value to 1.
					for(j = 0; j < numColumns; j++)
					{
						//Modify left matrix.
						Left_Mat->SetVal(i, j, Left_Mat->GetVal(i, j) / var);

						//Modify right matrix.
						Right_Mat->SetVal(i, j, Right_Mat->GetVal(i, j) / var);
					}
				}
				
				//Make values in other rows above or below this diagonal value to 0.
				//That is, eliminating each row.
				for(j = 0; j < numRows; j++)
				{
					//Avoid checking itself and unnecessary calculations.
					if(i != j && !IsEqual(Left_Mat->GetVal(j, i), 0.0f))
					{
						var = -Left_Mat->GetVal(j, i);
						for(k = 0; k < numColumns; k++)
						{
							//Modify left matrix.
							Left_Mat->SetVal(j, k, Left_Mat->GetVal(j, k) + Left_Mat->GetVal(i, k) * var);
							//Modify right matrix.
							Right_Mat->SetVal(j, k, Right_Mat->GetVal(j, k) + Right_Mat->GetVal(i, k) * var);
						}
					}//End eliminating one row.
				}
		}

	}

	if(Left_Mat != NULL)
	{
		delete Left_Mat;
		Left_Mat = NULL;
	}

	return Right_Mat;
}

void Matrix::SetToIdentity()
{
	int i;

	for(i = 0; i < numRows * numColumns; i++)
		m[i] = 0.0f;

	for(i = 0; i < numRows; i++)
		SetVal(i, i, 1.0f);
}

void Matrix::ExchangeRow(int Row1, int Row2)
{
	int i;
	float *swap = new float[numColumns];

	//Swap values in Row1 first and set values in Row1 from Row2.
	for(i = 0; i < numColumns; i++)
	{
		swap[i] = GetVal(Row1, i);
		SetVal(Row1, i, GetVal(Row2, i));
	}

	//Set Row2's values from swapping array.
	for(i = 0; i < numColumns; i++)
		SetVal(Row2, i, swap[i]);

	if(swap != NULL)
	{
		delete[] swap;
		swap = NULL;
	}
}

bool Matrix::IsEqual(float var1, float var2)
{
	//Precison up to 10 of power -5.
	if((float)fabs((float)(var1 - var2)) < 1e-5)
		return true;
	else
		return false;
}

void Matrix::ResetDimension(int Row, int Column)
{
	if(m != NULL)
	{
		delete[] m;
		m = NULL;
	}

	m = new float[Row * Column];
	numRows = Row;
	numColumns = Column;

	for(int i = 0; i < Row * Column; i++)
		m[i] = 0.0f;
}

bool Matrix::Product(const Matrix *mat_in, Matrix *result)
{
	if(numColumns == mat_in->numRows)
	{
		int i, j, k, var;
		float temp = 0.0f;
		Matrix temp_in;

		temp_in = *mat_in;

		result->ResetDimension(numRows, temp_in.numColumns);

		for(k = 0; k < numRows; k++)
		{
			var = (temp_in.numColumns) * k;
			for(i = 0; i < temp_in.numColumns; i++)
			{
				for(j = 0; j < numColumns; j++)
					temp += m[(numColumns) * k + j] * temp_in.m[(temp_in.numColumns) * j + i];

				result->m[var + i] = temp;

				temp = 0.0f;
			}
		}
	}
	else
		return false;

	return true;
}

bool Matrix::ScalarProduct(float var)
{
	for(int i = 0; i < numRows * numColumns; i++)
		m[i] = m[i] * var;	

	return true;
}

bool Matrix::EqualTo(const Matrix *mat_in)
{
	ResetDimension(mat_in->numRows, mat_in->numColumns);
	for(int i = 0; i < numRows * numColumns; i++)
		m[i] = mat_in->m[i];

	return true;
}

bool Matrix::Add(Matrix *pMat)
{
	if(numRows != pMat->numRows || numColumns != pMat->numColumns)
		return false;

	unsigned int i, total = numRows * numColumns;

	for(i = 0; i < total; i++)
		m[i] = m[i] + pMat->m[i];

	return true;
}

bool Matrix::Transpose()
{
	Matrix TempMat;
	int i, j;

	TempMat = *this;
	
	ResetDimension(TempMat.numColumns, TempMat.numRows);

	numRows = TempMat.numColumns;
	numColumns = TempMat.numRows;

	for(i = 0; i < numRows; i++)
	{
		for(j = 0; j < numColumns; j++)
			SetVal(i, j, TempMat.GetVal(j, i));
	}

	return true;
}

bool Matrix::Subtract(Matrix *pMat)
{
	if(numRows != pMat->numRows || numColumns != pMat->numColumns)
		return false;

	unsigned int i, total = numRows * numColumns;

	for(i = 0; i < total; i++)
		m[i] = m[i] - pMat->m[i];	

	return true;
}

bool Matrix::ToDiagonal()
{
	if(numRows != numColumns)
		return false;

	int i, j;

	for(i = 0; i < numRows; i++)
	{
		for(j = 0; j < numColumns; j++)
		{
			if(i != j)
				SetVal(i, j, 0.0f);
		}
	}

	return true;
}
