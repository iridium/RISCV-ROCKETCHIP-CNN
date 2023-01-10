#include <stdio.h>
#include <stdint.h>
#include "lowrisc_memory_map.h"
#include "mini-printf.h"

char text[] = "Vafgehpgvba frgf jnag gb or serr!" ;

void foo(){
	int i=0;
	while(text[i]){
		char lower = text[i] | 32;
		if ( lower >= 'a' && lower <= 'm' )
			text[i] += 13;
		else if ( lower > 'm' && lower <= 'z')
			text[i] -= 13;
		i++;
	}
}

volatile int wait = 1;
int main(int argc , char ** argv) {

	printf("Welcome to this test application written to document the lowrisc platform:");
	
	while(wait);
	
	printf("Old text: %s \n",text);
	foo();
	printf("New text: %s \n", text);
	
	while(!wait);
	
	return 0;
}