                     MAIN
                       │
      ┌────────────────┴───────────────┐
      │                                │
      ▼                                ▼
  Création                     Création
  des coders                   des dongles
      │                                │
      └──────────────┬─────────────────┘
                     │
                     ▼
            Lancement threads
                     │
     ┌───────────────┼───────────────┐
     │               │               │
     ▼               ▼               ▼
   Coder 1        Coder 2         Coder N
     │               │               │
     └───── utilise les dongles ─────┘
                     │
                     ▼
               Scheduler
              FIFO / EDF
                     │
                     ▼
                Priority Queue
                     │
                     ▼
                  Dongles
                     │
                     ▼
                  Monitor
                     │
          burnout ou fin quota
                     │
                     ▼
                   STOP
