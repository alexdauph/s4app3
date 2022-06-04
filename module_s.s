/************************************
 * Name :       module_s
 * Author:      Alexabdre Dauphinais
 * Date :       E22
 ************************************/

.set noreorder  
    
.data
   
.text

.global module_s
# début de la fonction
.ent module_s
module_s:			            # Étiquette de la fonction
    
    li $s1, 1			# Toujours 1 pour faciliter des calculs
    mthi $0                     # Init HI à 0
    mtlo $0                     # init LO à 0
    
    madd $a0, $a0               # Ajouter x^2
    madd $a1, $a1               # Ajouter y^2
    madd $a2, $a2               # Ajouter z^2			
    mflo $s0			# Copier S (x^2 + y^2 + z^2) dans $s0
    srl $v0, $s0, 1		# Estimé initial dans $v0
    move $t1, $v0		# Init $t1 à l'estimé
    
    loop : 
	move $t0, $s0		# Copier S dans $t0
	div $t0, $v0		# S / x(n-1)
	madd $v0, $s1		# x(n-1) + S/x(n-1)
	mflo $v0		# Copier x(n-1) + S/x(n-1)
	srl $v0, $v0, 1		# 1/2 * (x(n-1) + S/x(n-1))
    
	sub $t2, $t1, $v0	# 
	bgt $t2, $s1, loop	# if(($v0 - $t1) <= 1)
	move $t1, $v0		# Copier x(n-1)
	 
    jr $ra		                # Retour à la fonction
    nop			                # delay slot
# fin de la fonction   
.end module_s
