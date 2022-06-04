/************************************
 * Name :     pmod.s
 * Author:    Jp Gouin
 * Version :  h2022
 ************************************/

.set noreorder  
    
.data
   
.text

.global pmod_s
# début de la fonction
.ent pmod_s
pmod_s:			# Étiquette de la fonction
    li $t0, 0x0200	# Création du masque pour RD9 (10eme bit à 1) 
			# 0x0200 = 0b 0000 0010 0000 0000
    lw $t1, PORTD($0)	# Lecture du port D
    XOR $t2, $t1, $t0	# Masquage
    sw $t2, LATD($0)	# Écriture du port D
  
    jr $ra		# Retour à la fonction
    nop			# delay slot
# fin de la fonction   
.end pmod_s
