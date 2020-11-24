///
/// @brief      Implementations for time and duration classes
///

#include "STF/Interface/Types/STFTime.h"

STFLoPrec32BitDuration MAX_LO_PREC_32_BIT_DURATION(0x7fffffff, STFTU_LOWSYSTEM);
STFLoPrec32BitDuration MIN_LO_PREC_32_BIT_DURATION(0x80000000, STFTU_LOWSYSTEM);

STFHiPrec32BitDuration MAX_HI_PREC_32_BIT_DURATION(0x7fffffff, STFTU_108MHZTICKS);
STFHiPrec32BitDuration MIN_HI_PREC_32_BIT_DURATION(0x80000000, STFTU_108MHZTICKS);

STFHiPrec64BitDuration MAX_HI_PREC_64_BIT_DURATION(STFInt64(0xffffffff, 0x7fffffff), STFTU_108MHZTICKS);
STFHiPrec64BitDuration MIN_HI_PREC_64_BIT_DURATION(STFInt64(0x00000000, 0x80000000), STFTU_108MHZTICKS);

STFHiPrec64BitTime MAX_HI_PREC_64_BIT_TIME(STFInt64(0xffffffff, 0x7fffffff), STFTU_108MHZTICKS);
STFHiPrec64BitTime MIN_HI_PREC_64_BIT_TIME(STFInt64(0x00000000, 0x80000000), STFTU_108MHZTICKS);

STFLoPrec32BitDuration ZERO_LO_PREC_32_BIT_DURATION(0x00000000, STFTU_108MHZTICKS);
STFHiPrec32BitDuration ZERO_HI_PREC_32_BIT_DURATION(0x00000000, STFTU_108MHZTICKS);
STFHiPrec64BitDuration ZERO_HI_PREC_64_BIT_DURATION(0x00000000, STFTU_108MHZTICKS);
STFHiPrec64BitTime     ZERO_HI_PREC_64_BIT_TIME(STFInt64(0x00000000, 0x00000000), STFTU_108MHZTICKS);

// Scale
STFHiPrec32BitDuration STFHiPrec32BitDuration::Scale(const int32 from, const int32 to) const
   {
   uint32   res_u, res_l, upper, lower, udur, uto, ufrom;
   bool     sign = false;
   
   // Make sure that the divisor is NOT zero
   ASSERT(from != 0);
   if (from == 0) return STFHiPrec32BitDuration(0x7fffffff, STFTU_108MHZTICKS);

	udur = this->duration;

   // evaluate sign of the result and continue the calculation with unsigned integers. 
	if (udur & 0x80000000) 
		{
		udur = ~udur + 1;		
		sign = true;
		}      

   if (to < 0)
      {
      uto = -to;
      sign = !sign;
      }
   else
      {
      uto = to;
      }

   if (from < 0)
      {
      ufrom = -from;
      sign = !sign;
      }
   else
      {
      ufrom = from;
      }

   // Do the multiplication
   MUL32x32(udur, uto, upper, lower);
   
   // Do the division
   if (upper < ufrom)
      {
      res_u = 0;
      res_l = DIV64x32(upper, lower, ufrom);
      }
   else
      {
      res_u = upper / ufrom;
      res_l = DIV64x32( (upper % ufrom), lower, ufrom);
      }

   //return result depending on sign and possible saturation
   if (!sign) //positive case
      {
      if (res_u != 0 || res_l > 0x7fffffff) //positive saturation
         return STFHiPrec32BitDuration(0x7fffffff, STFTU_108MHZTICKS);
      else //positive value
         return STFHiPrec32BitDuration(res_l, STFTU_108MHZTICKS);
      }
   else //negative case
      {
      if (res_u != 0 || res_l > 0x80000000) //negative saturation
         return STFHiPrec32BitDuration(0x80000000, STFTU_108MHZTICKS);
      else //negative value
         return STFHiPrec32BitDuration(-(int32)res_l, STFTU_108MHZTICKS);
      }
   }


// FractMul
STFHiPrec32BitDuration STFHiPrec32BitDuration::FractMul(const int32 fract) const
   {
   uint32   res_l, res_u, udur, ufract;
   bool     sign = false;

	udur = this->duration;

   // evaluate sign of the result and continue the multiplication with unsigned integers.
   if (udur & 0x80000000) 
		{
		udur = ~udur + 1;		
		sign = true;
		}   	
   
   if (fract < 0)
      {
      ufract = -fract;
      sign = !sign;
      }
   else
      {
      ufract = fract;
      }

   // Do the multiplication
   MUL32x32(udur, ufract, res_u, res_l);
   
   // Shift result to compensate 16.16 fract
   res_l = (res_u << (32 - 16)) + (res_l >> 16);
   res_u = res_u >> 16;
   
   //return result depending on sign and possible saturation
   if (!sign) //positive case
      {
      if (res_u != 0 || res_l > 0x7fffffff) //positive saturation
         return STFHiPrec32BitDuration(0x7fffffff, STFTU_108MHZTICKS);
      else //positive value
         return STFHiPrec32BitDuration(res_l, STFTU_108MHZTICKS);
      }
   else //negative case
      {
      if (res_u != 0 || res_l > 0x80000000) //negative saturation
         return STFHiPrec32BitDuration(0x80000000, STFTU_108MHZTICKS);
      else //negative value
         return STFHiPrec32BitDuration(-(int32)res_l, STFTU_108MHZTICKS);
      }
   }


// FractDiv
STFHiPrec32BitDuration STFHiPrec32BitDuration::FractDiv(const int32 fract) const
   {
   uint32   res_l, res_u, lower, upper, udur, ufract;
   bool     sign = false;

   // Make sure that the divisor is NOT zero
   ASSERT(fract != 0);
   if (fract == 0) return STFHiPrec32BitDuration(0x7fffffff, STFTU_108MHZTICKS);

	udur = this->duration;	

   // evaluate sign of the result and continue the division with unsigned integers.
	if (udur & 0x80000000) 
		{
		udur = ~udur + 1;		
		sign = true;
		}      
   
   if (fract < 0)
      {
      ufract = -fract;
      sign = !sign;
      }
   else
      {
      ufract = fract;
      }

   // shift the duration to compensate 16.16 fract
   upper = udur >> 16;
   lower = udur << 16;

   // Do the division
   if (upper < ufract)
      {
      res_u = 0;
      res_l = DIV64x32(upper, lower, ufract);
      }
   else
      {
      res_u = upper / ufract;
      res_l = DIV64x32( (upper % ufract), lower, ufract);
      }
   
   //return result depending on sign and possible saturation
   if (!sign) //positive case
      {
      if (res_u != 0 || res_l > 0x7fffffff) //positive saturation
         return STFHiPrec32BitDuration(0x7fffffff, STFTU_108MHZTICKS);
      else //positive value
         return STFHiPrec32BitDuration(res_l, STFTU_108MHZTICKS);
      }
   else //negative case
      {
      if (res_u != 0 || res_l > 0x80000000) //negative saturation
         return STFHiPrec32BitDuration(0x80000000, STFTU_108MHZTICKS);
      else //negative value
         return STFHiPrec32BitDuration(-(int32)res_l, STFTU_108MHZTICKS);
      }
   }

// Scale
STFHiPrec64BitDuration STFHiPrec64BitDuration::Scale(const int32 from, const int32 to) const
   {
   uint32   res_l, res_m, res_u, lower, upper, middle, m1, m2, udur_u, udur_l, uto, ufrom;
   bool     sign = false;

   // Make sure that the divisor is NOT zero
   ASSERT(from != 0);
   if (from == 0) return STFHiPrec64BitDuration(STFInt64(0xffffffff,0x7fffffff), STFTU_108MHZTICKS);
	
	udur_u = this->duration.Upper();
   udur_l = this->duration.Lower();			  

   // evaluate sign of the result and continue the calculation with unsigned integers.
	if (udur_u & 0x80000000) 
		{
		udur_l = ~udur_l + 1;
		udur_u = ~udur_u + (udur_l ? 0 : 1);
		sign = true;
		}   

   if (to < 0)
      {
      uto = -to;
      sign = !sign;
      }
   else
      {
      uto = to;
      }

   if (from < 0)
      {
      ufrom = -from;
      sign = !sign;
      }
   else
      {
      ufrom = from;
      }

   
   // Do the multiplication
   MUL32x32(udur_l, uto, m1, lower);
   MUL32x32(udur_u, uto, upper, m2);
   middle = m1 + m2;
   if (middle < m1) upper++;

   
   // Do the division
   res_u = upper / ufrom;
   
   m1 = (upper << 16) + (middle >> 16);
   res_m = m1 / ufrom;
   
   m2 = ((m1 % ufrom) << 16) + ((middle << 16) >> 16);
   res_m = (res_m << 16) + m2 / ufrom;
   
   res_l = DIV64x32((m2 % ufrom), lower, ufrom);

   //return result depending on sign and possible saturation
   if (!sign) //positive case
      {
      if (res_u != 0 || res_m > 0x7fffffff) //positive saturation
         return STFHiPrec64BitDuration(STFInt64(0xffffffff,0x7fffffff), STFTU_108MHZTICKS);
      else //positive value
         return STFHiPrec64BitDuration(STFInt64(res_l, res_m), STFTU_108MHZTICKS);
      }
   else //negative case
      {
      if (res_u != 0 || res_m > 0x80000000) //negative saturation
         return STFHiPrec64BitDuration(STFInt64(0x00000000, 0x80000000), STFTU_108MHZTICKS);
      else //negative value
         return STFHiPrec64BitDuration(- STFInt64(res_l, res_m), STFTU_108MHZTICKS);
      }
   }


// FractMul
STFHiPrec64BitDuration STFHiPrec64BitDuration::FractMul(const int32 fract) const
   {
   uint32   res_l, res_m1, res_m2, res_m, res_u, udur_u, udur_l, ufract;
   bool     sign = false;

	udur_u = this->duration.Upper();
   udur_l = this->duration.Lower();

   // evaluate sign of the result and continue the multiplication with unsigned integers.
   if (udur_u & 0x80000000) 
		{
		udur_l = ~udur_l + 1;
		udur_u = ~udur_u + (udur_l ? 0 : 1);
		sign = true;
		}	
   
   if (fract < 0)
      {
      ufract = -fract;
      sign = !sign;
      }
   else
      {
      ufract = fract;
      }

   // Do the multiplication
   MUL32x32(udur_l, ufract, res_m1, res_l);
   MUL32x32(udur_u, ufract, res_u, res_m2);
   res_m = res_m1 + res_m2;
   if (res_m < res_m1) res_u++;
   
   // Shift result to compensate 16.16 fract
   res_l = (res_m << (32 - 16)) + (res_l >> 16);
   res_m = (res_u << (32 - 16)) + (res_m >> 16);
   res_u = res_u >> 16;
   
   //return result depending on sign and possible saturation
   if (!sign) //positive case
      {
      if (res_u != 0 || res_m > 0x7fffffff) //positive saturation
         return STFHiPrec64BitDuration(STFInt64(0xffffffff,0x7fffffff), STFTU_108MHZTICKS);
      else //positive value
         return STFHiPrec64BitDuration(STFInt64(res_l, res_m), STFTU_108MHZTICKS);
      }
   else //negative case
      {
      if (res_u != 0 || res_m > 0x80000000) //negative saturation
         return STFHiPrec64BitDuration(STFInt64(0x00000000, 0x80000000), STFTU_108MHZTICKS);
      else //negative value
         return STFHiPrec64BitDuration(- STFInt64(res_l, res_m), STFTU_108MHZTICKS);
      }
   }


// FractDiv
STFHiPrec64BitDuration STFHiPrec64BitDuration::FractDiv(const int32 fract) const
   {
   uint32   res_l, res_m, res_u, lower, middle, upper, m1, m2, udur_u, udur_l, ufract;
   bool     sign = false;

   // Make sure that the divisor is NOT zero
   ASSERT(fract != 0);
   if (fract == 0) return STFHiPrec64BitDuration(STFInt64(0xffffffff,0x7fffffff), STFTU_108MHZTICKS);

	udur_u = this->duration.Upper();
   udur_l = this->duration.Lower();

   // evaluate sign of the result and continue the division with unsigned integers.
   if (udur_u & 0x80000000) 
		{
		udur_l = ~udur_l + 1;
		udur_u = ~udur_u + (udur_l ? 0 : 1);
		sign = true;
		}
   
   if (fract < 0)
      {
      ufract = -fract;
      sign = !sign;
      }
   else
      {
      ufract = fract;
      }

   // shift the duration to compensate 16.16 fract
   upper = udur_u >> 16;
   middle = (udur_u << 16) + (udur_l >> 16);
   lower = udur_l << 16;

   
   // Do the division
   res_u = upper / ufract;
   
   m1 = (upper << 16) + (middle >> 16);
   res_m = m1 / ufract;
   
   m2 = ((m1 % ufract) << 16) + ((middle << 16) >> 16);
   res_m = (res_m << 16) + m2 / ufract;
   
   res_l = DIV64x32((m2 % ufract), lower, ufract);
   

   //return result depending on sign and possible saturation
   if (!sign) //positive case
      {
      if (res_u != 0 || res_m > 0x7fffffff) //positive saturation
         return STFHiPrec64BitDuration(STFInt64(0xffffffff,0x7fffffff), STFTU_108MHZTICKS);
      else //positive value
         return STFHiPrec64BitDuration(STFInt64(res_l, res_m), STFTU_108MHZTICKS);
      }
   else //negative case
      {
      if (res_u != 0 || res_m > 0x80000000) //negative saturation
         return STFHiPrec64BitDuration(STFInt64(0x00000000, 0x80000000), STFTU_108MHZTICKS);
      else //negative value
         return STFHiPrec64BitDuration(- STFInt64(res_l, res_m), STFTU_108MHZTICKS);
      }
    
   }

// FractMul
STFHiPrec64BitTime STFHiPrec64BitTime::FractMul(const int32 fract) const
   {
   uint32   res_l, res_m1, res_m2, res_m, res_u, udur_u, udur_l, ufract;
   bool     sign = false;

	udur_u = this->time.Upper();
   udur_l = this->time.Lower();

	
	if (udur_u & 0x80000000) 
		{
		udur_l = ~udur_l + 1;
		udur_u = ~udur_u + (udur_l ? 0 : 1);
		sign = true;
		}
   
   if (fract < 0)
      {
      ufract = -fract;
      sign = !sign;
      }
   else
      {
      ufract = fract;
      }

   // Do the multiplication
   MUL32x32(udur_l, ufract, res_m1, res_l);
   MUL32x32(udur_u, ufract, res_u, res_m2);
   res_m = res_m1 + res_m2;
   if (res_m < res_m1) res_u++;
   
   // Shift result to compensate 16.16 fract
   res_l = (res_m << (32 - 16)) + (res_l >> 16);
   res_m = (res_u << (32 - 16)) + (res_m >> 16);
   res_u = res_u >> 16;
   
   //return result depending on sign and possible saturation
   if (!sign) //positive case
      {
      if (res_u != 0 || res_m > 0x7fffffff) //positive saturation
         return STFHiPrec64BitTime(STFInt64(0xffffffff,0x7fffffff), STFTU_108MHZTICKS);
      else //positive value
         return STFHiPrec64BitTime(STFInt64(res_l, res_m), STFTU_108MHZTICKS);
      }
   else //negative case
      {
      if (res_u != 0 || res_m > 0x80000000) //negative saturation
         return STFHiPrec64BitTime(STFInt64(0x00000000, 0x80000000), STFTU_108MHZTICKS);
      else //negative value
         return STFHiPrec64BitTime(- STFInt64(res_l, res_m), STFTU_108MHZTICKS);
      }
   }

