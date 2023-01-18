Voicila Alexandra, 335CB

Initial, pentru procesele coordonator, am citit din fisierele specifice si am
populat vectorul de workeri al fiecaruia.
Apoi, am abordat problema in functie de rank-ul fiecarui proces. 
Pentru primul task, procesul 0 deschide inelul. Ma folosesc de 2 vectori principali, 
contor (care are dimensiune 4 si contine numarul de workeri pentru fiecare coordonator)
si final (care contine indicii worker-ilor) care sunt populati progresiv, fiind trimisi
pe calea 0->1->2->3. Procesul 3 afla topologia finala primul si o trimite worker-ilor 
sai. Afisarea se face prin prelucrarea celor 2 vectori in functia printTopology. 
Topologia este trimisa inapoi catre toate procesele pe calea 3->2->1->0.

Pentru task-ul 2, procesul coordonator 0 creeaza vectorul initial si il trimite catre
coordonatori pe aceesi cale ca topologia initiala, impreuna cu dimensiunea sa. 
Primind vectorul, fiecare coordonator trimite worker-ilor sai doar partea pe care ei
trebuie sa o prelucreze, folosind functia sendPartOfVector, ce aplica formulele din
laborator. Apoi fiecare proces coordonator primeste aceste parti prelucrate si le 
asambleaza pe parcurs intr-un vector care ajunge la coordonatorul 0. Acesta sorteaza 
vectorul descrescator si face afisarea lui.

Pentru task-ul 3, m-am folosit de 2 map-uri care asociaza un nod din inel cu succesorul
sau. Astfel pentru inel complet, folosind abordarea de la primul task, ordinea din
inel va fi 0->1->2->3. Iar pentru task-ul 3, ordinea se inverseaza: 0->3->2->1. Folosesc
functia findKey pentru cazurile de intoarcere de pachete.

Dupa fiecare functie de trimitere, afisez mesajele corespunzatoare.
