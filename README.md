# 🧁 Prova Finale: Progetto di Algoritmi e Strutture Dati: Simulatore di Pasticceria Industriale

Questo progetto è una simulazione a tempo discreto del funzionamento di una pasticceria industriale, sviluppato come prova finale di **Algoritmi e Strutture Dati 2023-2024** presso il Politecnico di Milano. Il programma gestisce ingredienti, ricette, ordini dei clienti, rifornimenti di magazzino e spedizioni tramite corriere.

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

## 🔢 Requisiti di valutazione

- Il progetto viene valutato attraverso un testbench privato che inietta un input nel file `stdin` e analizza l'output prodotto su `stdout`. Se tale output è corretto, viene valutata la durata dell'elaborazione e l'impiego massimo di memoria.
- La prova è stata passata con 30L/30.

| VOTO | TEMPO  | MEMORIA    |
|------|--------|------------|
| OPEN | 45.0 s | 150.0 MiB  |
| 18   | 14.0 s | 35.0 MiB   |
| 21   | 11.5 s | 30.0 MiB   |
| 24   | 9.0 s  | 25.0 MiB   |
| 27   | 6.5 s  | 20.0 MiB   |
| 30   | 4.0 s  | 15.0 MiB   |
| 30L  | 1.5 s  | 14.0 MiB   |

NB: il test di OPEN è stato inserito con limiti di tempo e memoria volutamente molto larghi per dare un'idea agli studenti che non avessero passato il test del 18 di capire di quando avessero ecceduto le richieste.

L'ultima sottoposizione di questo file ha impiegato **1.208 s** per l'elaborazione e ha impiegato **12.6 MiB** di memoria nel suo picco.

## 🔧 Compilazione

Per compilare il programma:
```bash
gcc -o pasticceria Progetto.c
```
NB: è presente un file `Makefile` che imposta alcuni flag di compilazione forniti da specifica. Il compilatore considera tutti i *warning* come se fossero degli errori.

## ▶️ Esecuzione

Per eseguire il programma con input da file:
```bash
./pasticceria < input.txt >output.txt
```
