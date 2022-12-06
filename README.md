# KST32B vector font viewer for Wio Terminal

## Structure

```mermaid
flowchart LR
    id1[KST32B_viewer]-->id2[KST32B_draw]
    id1-->id3[plotter]
    id1-->id4[KST32B_plot]
    id1-->id5[hpgl]
    id2-->id7[Display]
    id3-->id6[swSerial]
    id4-->id3
    id4-->id6
    id5-->id6
```

## License

(TODO)
