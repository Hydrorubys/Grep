# Grep
Ma version de grep assez rudimentaire (absolument pas opti)

## Comment l'utiliser ?

Très simple, appeler ./mygrep 'regex' monfichier.txt

/!\ La regex doit être post-fixée, les symboles clés sont : 
- @ pour la concaténation
- | pour l'union
- * pour l'étoile
- ? pour "caractère facultatif"
- . pour un caractère quelconque

Et ces symbole ne doivent pas se retrouver dans la motif recherché (pas de symbole d'échappement tel que \@)
