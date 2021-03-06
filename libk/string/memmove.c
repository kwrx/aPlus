/*                                                                      
 * GPL3 License                                                         
 *                                                                      
 * Author(s):                                                              
 *      Antonino Natale <antonio.natale97@hotmail.com>                  
 *                                                                      
 *                                                                      
 * Copyright (c) 2013-2019 Antonino Natale                              
 *                                                                      
 * This file is part of aPlus.                                          
 *                                                                      
 * aPlus is free software: you can redistribute it and/or modify        
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 3 of the License, or    
 * (at your option) any later version.                                  
 *                                                                      
 * aPlus is distributed in the hope that it will be useful,             
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        
 * GNU General Public License for more details.                         
 *                                                                      
 * You should have received a copy of the GNU General Public License    
 * along with aPlus.  If not, see <http://www.gnu.org/licenses/>.       
 */                                                                     
                                                                        
#include <stdint.h>
#include <string.h>
#include <string.h>
#include <sys/types.h>




void *memmove(void* dest, const void* src, size_t n) {

	unsigned char* d = (unsigned char*) dest;
	unsigned char* s = (unsigned char*) src;


	if((uintptr_t) dest < (uintptr_t) src) {

		for(size_t i = 0; i < n; i++)
			d[i] = s[i];

	} else {

		for(size_t i = 0; i < n; i++)
			d[n - (i + 1)] = s[n - (i + 1)];

	}

	return dest;

}