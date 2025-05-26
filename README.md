# 🧁 Prova Finale: Progetto di Algoritmi e Strutture Dati: Simulatore di Pasticceria Industriale

Questo progetto è una simulazione a tempo discreto del funzionamento di una pasticceria industriale, sviluppato come prova finale del corso di **Algoritmi e Strutture Dati 2023-2024**. Il programma gestisce ingredienti, ricette, ordini dei clienti, rifornimenti di magazzino e spedizioni tramite corriere.

## ⚙️ Funzionalità principali

- **Gestione ricette**: aggiunta, rimozione e utilizzo di ricette per dolci.
- **Gestione magazzino**: stoccaggio di ingredienti con gestione della scadenza.
- **Gestione ordini**: accettazione e preparazione ordini in base alla disponibilità degli ingredienti.
- **Gestione corriere**: carico periodico degli ordini pronti secondo peso e capienza.

## 🕓 Simulazione a tempo discreto

Ogni comando in ingresso corrisponde a un'unità di tempo. Alcuni eventi (come il passaggio del corriere) avvengono a intervalli regolari predefiniti.

## 💬 Comandi supportati

Il programma legge i comandi da **stdin**, uno per riga, con il seguente formato:

- `aggiungi_ricetta <nome_ricetta> <ingrediente> <quantità> ...`
- `rimuovi_ricetta <nome_ricetta>`
- `rifornimento <ingrediente> <quantità> <scadenza> ...`
- `ordine <nome_ricetta> <numero_dolci>`

## 📤 Output

Oltre alla risposta immediata per ciascun comando (`aggiunta`, `rifornito`, `accettato`, ecc.), ad ogni istante multiplo della periodicità del corriere viene stampato lo stato del carico del camioncino.

Esempio di output:
```
5 ciambella 6
10 torta 1
13 torta 1
```

## 🚚 Regole di spedizione

- Gli ordini vengono caricati nel camioncino in ordine cronologico, finché non si raggiunge il limite di peso.
- L’ordine di carico effettivo è per peso decrescente, a parità di peso vale l’ordine cronologico.

## 🔧 Compilazione

Per compilare il programma:
```bash
gcc -o pasticceria progetto.c
```

## ▶️ Esecuzione

Per eseguire il programma con input da file:
```bash
./pasticceria < input.txt
```

## 🧪 Esempio

Input:
```
5 325
aggiungi_ricetta torta farina 50 uova 10 zucchero 20
rifornimento farina 100 10 uova 100 10 zucchero 100 10
ordine torta 2
```

Output:
```
aggiunta
rifornito
accettato
5 0 torta 2
```