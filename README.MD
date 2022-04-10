# RENDU FINAL
## Présentation de l'haxanôme
Notre hexanôme est constitué de 7 membres :
Pauline Vilpini, Louis Lombard, Paula Schreiber, Oskar Howe, Dimitri Jeannon, Alexis Brissard, Thibaud Pires et Mohammed Nassik.

Notre organisation était très horizontale et nous avons beacoup travaillé par binomes en peer programming. Nous avons chacun aisnsi pu travailler sur la partie du projet qui nous convenait le plus et chacun a pu y trouver son compte.
## Utilisation
Pour build le compilateur, voir le README.md dans le dossier "compiler".

Une fois l'executable ifcc généré, il faut l'exécuter avec le nom du fichier à parser en argument. 
```bash 
./ifcc <src.c>
```
Par défaut, le code généré est affiché dans la console. Pour générer un fichier .s utilisez les focntionnalitées bash.
```bash
./ifcc <src.c> > <asm.s>
```
## Fonctionnalités implémentées:
- Type de données:
    - int
    - char
- Déclaration de variables:
    ```C
    int x = 8, y = 7, z;
    ```
- Assignation de valeurs:
    ```C
    x = 8, y = 7;
    z = x + y;
    ```
    L'assignation renvoie bien la valeur assignée
- Opérations arithmétiques:
    - +, -, *, /, %
- Opérations logiques(*Les opérateurs ne sont pas lazy*):
    - &&, ||, !
- Comparaisons:
    - ==, !=, >, <, >=, <=
- Opérations bit à bit:
    - &, |, ^, ~, <<, >>
- Boucle while
- If-Then-Else:
    ```C
    if (x == y) {
        z = x + y;
    } else if(x > y) {
        z = x - y;
    }else {
        z = x * y;
    }
    ```
- Controle de boucle
    - break et continue
- Optimisations:
    ```C
    int x = 8 + 1 // movl $9, -4(%rbp)
    ```
- Warnings:
    - `-Wunused-variable`
    - `-Wuninitialized`
- Errors:
    - undeclared variable
    - redeclaration of variable
- Environnement de test:
    - Script modifié:
        - N'affiche que les tests ratés
        - `-f` pour ne lancer que les tests précédement ratés 
    - Tests précédés de "_" doivent échouer
