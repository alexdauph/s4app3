/************************************
 * Name :       pmod_s
 * Author:      Alexabdre Dauphinais
 * Date :       E22
 ************************************/

.set noreorder  
    
.data

    active:	    .word 0
    counter:    .word 0
    phase:      .word 0
    # ack_last:   .word 0
     
.text

.global pmod_s
# début de la fonction
.ent pmod_s
pmod_s:			                # Étiquette de la fonction

    li $s1, 1                   # Set $s1 to 1 for comparisons

    lw $t0, 0($a1)
    beq	$t0, $s1, reset	        # If pmod_new == 1 then reset
    nop
    j skip_reset                # If pmod_new != 1 then skip reset
    nop
    
    reset: 
        la $t0, active	     
        sw $s1, 0($t0)		    # set active to 1  
        la $t1, counter		     
        sw $0, 0($t1)           # reset counter      
        la $t1, phase		     
        sw $0, 0($t1)           # reset phase                     

    skip_reset:
        la $t0, active
        lw $t0, 0($t0)          # load active value   
        bne $t0, $s1, end       # check If transmitting is active
        nop
    
    parity:
        la $t0, counter
        lw $t0, 0($t0)              # Load counter value
	    sll $t0, $t0, 2		        # Multiply counter by 4 because 32 bytes
        add	$t0, $a0, $t0		    # $t0 = &pmod_data[counter]
        lw $s0, 0($t0)              # $s0 = pmod_data[counter]
        
        srl $t0, $s0, 2             # $t0 = $s0 >> 2
        xor $t0, $t0, $s0           # $t0 = $t0 ^ $s0
        srl $t1, $t0, 1             # $t1 = $t0 >> 1
        xor $t0, $t0, $t1           # $t0 = $t0 ^ $t1   
        andi $s2, $t0, 1            # $s2 = $t1 & 1         parity
    
    set_port:
        lw $t0, PORTC($0)	        # Read port C
	    li $t2, 0xFFFFFFE1
        and $t0, $t0, $t2           # Clear used bits for port C
        lw $t1, PORTG($0)           # Read port G
	    li $t2, 0xFFFFFFBF
        and $t1, $t1, $t2           # Clear used bits for port G
        # 
        andi $t2, $s0, 0b0001       # D0
        sll $t2, $t2, 2             # C2
        or $t0, $t0, $t2            # Set C2
        # 
        andi $t2, $s0, 0b0010       # D1
        or $t0, $t0, $t2            # Set C1
        # 
        andi $t2, $s0, 0b0100       # D2
        sll $t2, $t2, 2             # C4
        or $t0, $t0, $t2            # Set C4
        # 
        andi $t2, $s0, 0b1000       # D3
        sll $t2, $t2, 3             # G6
        or $t1, $t1, $t2            # Set G6
        # 
        sll $t2, $s2, 3             # parity
        or $t0, $t0, $t2            # Set C3 (partiy)
        # 
	    sw $t0, LATC($0)	        # Write port C
	    sw $t1, LATG($0)            # Write port G

        # JB1 (RC2)) pour D0            C2
        # JA2 (RC1)) pour D1            C1
        # JA3 (RC4)) pour D2            C4
        # JA4 (RG6)) pour D3            G6
        # JA4 (RC3)) pour parite        C3
        # JA4 (RC7)) pour ack           C7        

    branching:
        # la $t0, 
        lw $t0, phase($0)              # Load phase value 
        #
        li $t1, 0
        beq $t0, $t1, phase0        # Jump to phase0
        nop
        #
        li $t1, 1
        beq $t0, $t1, phase1        # Jump to phase1
        nop
        #
        li $t1, 2
        beq $t0, $t1, phase2        # Jump to phase2
        nop
        #
        li $t1, 3
        beq $t0, $t1, phase3        # Jump to phase3
        nop
        #
        li $t1, 4
        beq $t0, $t1, phase4        # Jump to phase4
        nop

    phase0:    
        lw $t0, PORTG($0)	        # Read port C
	    li $t2, 0xFFFFFF7F
        and $t0, $t0, $t2           # Clear used bits for STROBE
        # 
        sll $t2, $s1, 7             # STROBE
        or $t0, $t0, $t2            # Set C7
        # 
        sw $t0, LATG($0)	        # Write port C
        # 
        lw $t0, phase               # Load phase value
        addi $t0, $t0, 1            # Phase++
        la $t1, phase	            # Load phase address
        sw $t0, 0($t1)		        # Store phase
        #
        j end
        nop
     
    phase1:
        lw $t0, PORTG($0)	        # Read port C
	    li $t2, 0xFFFFFF7F
        and $t0, $t0, $t2           # Clear used bits for STROVE
        # 
        sw $t0, LATG($0)	        # Write port C
        # 
        lw $t0, phase               # Load phase value
        addi $t0, $t0, 1            # Phase++
        la $t1, phase	            # Load phase address
        sw $t0, 0($t1)		        # Store phase
        #
        j end
        nop
     
    phase2:
        li $v0, 1                   # Waiting for ACK
        #
        bne	$a2, $0, end	        # If ACK is high then exit
        nop
        #
        lw $t0, phase               # Load phase value
        addi $t0, $t0, 1            # Phase++
        la $t1, phase	            # Load phase address
        sw $t0, 0($t1)		        # Store phase
        #
        j end
        nop
     
    phase3:
        li $v0, 1                   # Waiting for ACK
        #
        bne	$a2, $s1, end	        # If ACK is low then exit
        nop
        #
        lw $t0, phase               # Load phase value
        addi $t0, $t0, 1            # Phase++
        la $t1, phase	            # Load phase address
        sw $t0, 0($t1)		        # Store phase
        #
        j end
        nop

    phase4:
        li $v0, 1                   # Waiting for ACK
        #
        bne	$a2, $0, end	        # If ACK is high then exit
        nop
        #
        li $v0, 0                   # Not waiting for ACK
        #
        la $t1, phase	            # Load phase address
        sw $0, 0($t1)		        # phase = 0
        #
        lw $t0, counter             # Load counter value
        addi $t0, $t0, 1            # Counter++
        la $t1, counter	            # Load counter address
        sw $t0, 0($t1)		        # Store counter
        #
        li $t1, 297                 # Max frame count
        bne $t0, $t1, end           # If counter != max then exit
        nop
        #
        la $t1, active	            # Load active address
        sw $0, 0($t1)		        # active = 0
        #
        j end
        nop
  
    end:
        jr $ra		# Retour à la fonction
        nop			# delay slot
# fin de la fonction   
.end pmod_s
