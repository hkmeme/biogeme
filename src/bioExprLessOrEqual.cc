//-*-c++-*------------------------------------------------------------
//
// File name : bioExprLessOrEqual.cc
// @date   Thu Apr 19 07:22:28 2018
// @author Michel Bierlaire
// @version Revision 1.0
//
//--------------------------------------------------------------------

#include "bioExprLessOrEqual.h"
#include <sstream>
#include "bioSmartPointer.h"
#include "bioDebug.h"
#include "bioExceptions.h"

bioExprLessOrEqual::bioExprLessOrEqual(bioSmartPointer<bioExpression>  l, bioSmartPointer<bioExpression>  r) :
  left(l), right(r) {
  listOfChildren.push_back(l) ;
  listOfChildren.push_back(r) ;

}

bioExprLessOrEqual::~bioExprLessOrEqual() {
}

bioSmartPointer<bioDerivatives>
bioExprLessOrEqual::getValueAndDerivatives(std::vector<bioUInt> literalIds,
					   bioBoolean gradient,
					   bioBoolean hessian) {

  theDerivatives = bioSmartPointer<bioDerivatives>(new bioDerivatives(literalIds.size())) ;

  if (gradient) {
    if (containsLiterals(literalIds)) {
      std::stringstream str ;
      str << "Expression LessOrEqual is not differentiable" << std::endl ; 
      throw bioExceptions(__FILE__,__LINE__,str.str()) ;
    }
    if (hessian) {
      theDerivatives->setDerivativesToZero() ;
    }
    else {
      theDerivatives->setGradientToZero() ;
    }
  }
  
  if (left->getValue() <= right->getValue()) {
    theDerivatives->f = 1.0 ;
  }
  else {
    theDerivatives->f = 0.0 ;
  }
  return theDerivatives ;
}

bioString bioExprLessOrEqual::print(bioBoolean hp) const {
  std::stringstream str ;
  if (hp) {
    str << "<=(" << left->print(hp) << "," << right->print(hp) << ")" ;
  }
  else {
    str << "(" << left->print(hp) << "<=" << right->print(hp) << ")" ;
  }
  return str.str() ;
}
