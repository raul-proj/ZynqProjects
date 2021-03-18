#ifndef SRC_HAL_CCU_ASSERT_H_
#define SRC_HAL_CCU_ASSERT_H_

#ifndef NDEBUG
/*****************************************************************************/
/**
* @brief    This assert macro is to be used for non void functions.
*
* @param    Expression: expression to be evaluated. If it evaluates to
*           true, the assert occurs.
*
* @return   If expresion is true, returns XST_FAILURE status
*
******************************************************************************/

#define AssertNonvoid(Expression)             		\
{                                                  	\
    if (Expression)									\
	{                              					\
    	xil_printf("Core #%d: Failure result found in function %s in file %s : line %d\r\n",\
				XPAR_CPU_ID, __FUNCTION__, __FILE__, __LINE__); \
        return XST_FAILURE;                         \
    }                                              	\
}
#else
#define AssertNonvoid(Expression)
#endif

#endif /* SRC_HAL_CCU_ASSERT_H_ */
