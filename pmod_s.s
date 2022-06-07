/************************************
 * Name :       pmod_s
 * Author:      Alexabdre Dauphinais
 * Date :       E22
 ************************************/

.set noreorder  
    
.data

    active:	    .word 0
    counter:    .word 0
    subcounter: .word 3
    ack_last:   .word 0
     
.text

.global pmod_s
# début de la fonction
.ent pmod_s
pmod_s:			                # Étiquette de la fonction

    li $s1, 1                   # Set $s1 to 1 for comparisons

    lw $t0, 0($a1)
    beq	$t0, $s1, reset	        # if pmod_new == 1 then reset
    nop
    j skip_reset                # if pmod_new != 1 then skip reset
    nop
    
    reset: 
        la $t0, active	     
        sw $s1, 0($t0)		    # set active to 1  
        la $t1, ack_last	     
        sw $0, 0($t1)		    # reset ack_last 
        la $t2, counter		     
        sw $0, 0($t2)           # reset counter
        li $t3, 2               
        la $t4, subcounter
        sw $t3, 0($t4)          # set subcounter to 2                         

    skip_reset:
        la $t0, active
        lw $t0, 0($t0)          # load active value   
        bne $t0, $s1, end       # check if transmitting is active
        nop
    
    
    
    lw $t0, PORTC($0)	# Lecture port C
    lw $t1, PORTG($0)   # Lecture port G
    
    
    
    
    
    
    
    # li $t0, 0x0200	# Création du masque pour RD9 (10eme bit à 1) 
			# 0x0200 = 0b 0000 0010 0000 0000
    
    # XOR $t2, $t1, $t0	# Masquage
    # sw $t2, LATD($0)	# Écriture du port D
  
    end:
        jr $ra		# Retour à la fonction
        nop			# delay slot
# fin de la fonction   
.end pmod_s
