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
    
    li $s1, 1			        # Toujours 1 pour faciliter des calculs
    li $s2, -1				# Toujours -1 pour faciliter des calculs
    mthi $0                     # Init HI à 0
    mtlo $0                     # init LO à 0
    
    madd $a0, $a0               # Ajouter x^2
    madd $a1, $a1               # Ajouter y^2
    madd $a2, $a2               # Ajouter z^2			
    mflo $s0			        # Copier S (x^2 + y^2 + z^2) dans $s0
    
    # Protection si total est 0
    beq $s0, $0, end		    # Si le total est 0
    li $v0, 0			        # Réponse égale 0

    # Estimation
    li $t1, 31			        # Utilisé pour soustraction
    clz $t0, $s0		        # Compter nombre de 0
    sub $t0, $t1, $t0		    # Donne la position du 1 le plus élevé
    srl $t0, $t0, 1		        # Division par 2
    sllv $v0, $s1, $t0		    # Estimé initial
    
    # Calcul
    loop :			
        move $t0, $s0		    # Copier S dans $t0
        div $t0, $v0		    # S / x(n-1)
        madd $v0, $s1		    # x(n-1) + S/x(n-1)
        mflo $v0		        # Copier x(n-1) + S/x(n-1)
        srl $v0, $v0, 1		    # 1/2 * (x(n-1) + S/x(n-1))
        
        sub $t2, $t0, $v0	    # Soustraction pour comparaison de fin
	
        move $t1, $v0		    # Copier x(n-1)
        beq $t2, $0, end	    # Si la différence est nulle
        nop                     
        beq $t2, $s1, end       # Si la différence est 1
        nop
	    beq $t2, $s2, end       # Si la différence est -1
        nop
        j loop                  # Retour au début de la boucle
        nop
        
    end :
        jr $ra		                # Retour à la fonction
        nop			                # delay slot
# fin de la fonction   
.end module_s
