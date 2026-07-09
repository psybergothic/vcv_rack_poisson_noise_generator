# Poisson Noise — VCV Rack plugin

A 6HP noise module for [VCV Rack 2](https://vcvrack.com/) that generates
**shot noise**: impulses whose timing follows a Poisson process. At each audio
sample, the number of events is drawn from a true Poisson distribution with
λ = rate × sample time (Knuth's algorithm). Low rates give sparse random
clicks (Geiger-counter style); high rates blend into dense crackle.

## Controls

| Control | Function |
| --- | --- |
| **RATE** | Mean event rate, exponential from 0.5 Hz to 16 kHz |
| **CV** input + trimpot | Modulates rate at 1 octave/volt (attenuverter) |
| **AMP** | Impulse amplitude, 0–10 V |
| **BI / UNI** | Bipolar (random polarity) or unipolar impulses |
| **TRIG** output | 1 ms, 10 V trigger on every event — clock other modules stochastically |
| **OUT** output | The noise signal |

## Building

Requires the [Rack SDK](https://vcvrack.com/manual/PluginDevelopmentTutorial):

```sh
export RACK_DIR=/path/to/Rack-SDK   # defaults to ~/Rack-SDK
make            # build plugin.so
make install    # build and install into your Rack user folder
```

## License

GPL-3.0-or-later
