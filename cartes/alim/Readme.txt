Check en 5 minutes chrono. 
Des pistes se touchent au niveau des composant (ex C18), il faut corriger tous les erreurs de type overlap (c'est vraiment l'erreur qu'il ne faut pas laisser si tu veux une carte fonctionnelle).
Très important : les masses de la tension isolée avec le traco like de celles des alim à découpage sont reliées... notre 5V propre ne l'est plus.
Pas de plan de masse -> en faire 2 (1 pour le 5V isolé, 1 pour le reste) ; il faut corriger le schematic pour renomer le GND du 5V isolé
Les pistes de l'entrée des batteries sont plus petites que celles des alim à découpage, ça devient useless...

Une seule résistance sur tous les micromatch aurait suffit, et pas besoin de faire des grosses pistes, c'est du signal, pas de la puissance

On peut diminuer la taille de la carte par 2
Le schematique n'est pas très compréhensible
