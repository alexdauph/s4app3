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
    
    mthi $0                     # Mettre registre haut de l'accumulateur à 0
    mtlo $0                     # Mettre registre bas de l'accumulateur à 0
    madd $a0, $a0               # Ajouter x^2
    madd $a1, $a1               # Ajouter y^2
    madd $a2, $a2               # Ajouter z^2

    mflo $t0			# Copier x^2 + y^2 + z^2 dans $t0

    mfhi $v1
    mflo $v0

    jr $ra		                # Retour à la fonction
    nop			                # delay slot
# fin de la fonction   
.end module_s
