#include "mbed.h"

#define fech 40000 // Fréquence d'échantillonnage souhaitée
#define TAILLE_BUFFER_SIG (fech/20) // Pour acquistion de 2000 echantillons

Serial pc(USBTX, USBRX);

/* Entrées sorties */
AnalogIn entree(PF_10); // A5 connecteur Arduino
PwmOut monpwm(PD_14); // D10 Arduino

/* Timer et Ticker */
Ticker read_sample_ticker ;

/* Interruptions */
InterruptIn button(USER_BUTTON);

/* Variables globales */
float moy = 0 ;
float eff_ACDC = 0 ;
float eff_AC = 0 ;
int pwmperiode = 500 ; //  us
int pwmpulsewidth = 250 ; //  us
float sig_in[TAILLE_BUFFER_SIG] ;

/* Programmes d'interruption */
void pressed()
{
    pwmpulsewidth +=  0.1*pwmperiode ;
    if (pwmpulsewidth > pwmperiode ) pwmpulsewidth = 0 ;
    monpwm.pulsewidth_us(pwmpulsewidth);
}

void read_sample() {
    static int i = 0 ;
    sig_in[i] = 3.3f*entree.read(); // Conversion en volts
    i++;
    if (i >= TAILLE_BUFFER_SIG) i = 0;
}

float val_moyenne() {
    int j ;
    float somme ;
    read_sample_ticker.detach(); 
    somme = 0 ;
    for (j = 0; j < TAILLE_BUFFER_SIG ; j++) 
        somme += sig_in[j] ;
    read_sample_ticker.attach_us(&read_sample,1000000/fech); 
    return(somme/TAILLE_BUFFER_SIG) ;
}

float val_efficace_ACDC() {
    int j ;
    float somme_carre = 0 ;
    read_sample_ticker.detach();     
    for (j = 0; j < TAILLE_BUFFER_SIG ; j++) 
        somme_carre += sig_in[j]*sig_in[j] ;
    read_sample_ticker.attach_us(&read_sample,1000000/fech);         
    return(sqrt(somme_carre/TAILLE_BUFFER_SIG)) ;
}

int main()
{
    button.rise(&pressed);
    pc.printf("\033[2J"); // Sequence escape pour effacer la console
    pc.printf("\033[0;0H"); // Curseur en 0 ; 0
    pc.printf("Mesure de valeur moyenne et efficace du signal analogique sur la broche PF_10(A5)\n");
    pc.printf("Calcul sur un bloc de %d echantillons\n",TAILLE_BUFFER_SIG); 
    pc.printf("Signal test interne disponible : PWM sur broche PD_14 (D10)\n");     
    pc.printf("Entrer la periode du signal PWM : 500 us < T < 50 000 us : ");
    pc.scanf("%d",&pwmperiode) ; pc.printf("\n");
    pwmpulsewidth = pwmperiode/2 ;
    monpwm.period_us(pwmperiode);
    monpwm.pulsewidth_us(pwmpulsewidth);
    pc.printf("Relier le signal analogique externe ou le signal test interne sur PF_10(A5)\n");
    pc.printf("Bouton USER pour modifier le rapport cyclique du signal test si necessaire\n");
    pc.printf("fech = %d Hz\n",fech);
    read_sample_ticker.attach_us(&read_sample,1000000/fech); 
    while (1) {
        pc.printf("\nFrequence signal test PWM = %d Hz Rapport cyclique = %4d %%\n",
        1000000/pwmperiode,(100*pwmpulsewidth)/pwmperiode);
        moy = val_moyenne() ;
        pc.printf("Valeur moyenne du signal sur PF_10(A5) = %10.3f volts\n",moy) ;
        eff_ACDC = val_efficace_ACDC() ;
        pc.printf("Valeur efficace AC+DC = %10.3f volts\n",eff_ACDC) ;    
        eff_AC = sqrt(eff_ACDC*eff_ACDC - moy*moy) ;
        pc.printf("Valeur efficace AC = %10.3f volts\n",eff_AC) ;  
        pc.printf("\033[5A"); // Sequence escape qui remonte le curseur de 5 lignes sur la console
        wait(0.2) ; // pause affichage
    }
}
