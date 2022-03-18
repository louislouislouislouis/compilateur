x // y → movl x, %eax; cltd; idivl y; %eax

x % y → movl x, %eax; cltd; idivl y; %edx

x \* y → movl x, %eax; imull y, %eax; %eax

x + y → movl x, %eax; addl y, %eax; %eax

x - y → movl x, %eax; subl y, %eax; %eax
