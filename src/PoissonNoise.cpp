#include "plugin.hpp"

struct PoissonNoise : Module {
	enum ParamId {
		RATE_PARAM,
		RATE_CV_PARAM,
		AMP_PARAM,
		MODE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RATE_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		NOISE_OUTPUT,
		TRIG_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		EVENT_LIGHT,
		LIGHTS_LEN
	};

	dsp::PulseGenerator trigPulse;

	PoissonNoise() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		// Knob value is the base-2 exponent of the rate: 2^-1 = 0.5 Hz up to 2^14 = 16384 Hz
		configParam(RATE_PARAM, -1.f, 14.f, 6.f, "Rate", " Hz", 2.f);
		configParam(RATE_CV_PARAM, -1.f, 1.f, 0.f, "Rate CV", " oct/V", 0.f, 1.f);
		configParam(AMP_PARAM, 0.f, 10.f, 5.f, "Amplitude", " V");
		configSwitch(MODE_PARAM, 0.f, 1.f, 1.f, "Polarity", {"Unipolar", "Bipolar"});
		configInput(RATE_INPUT, "Rate CV (1 oct/V)");
		configOutput(NOISE_OUTPUT, "Noise");
		configOutput(TRIG_OUTPUT, "Event trigger");
	}

	void process(const ProcessArgs& args) override {
		float pitch = params[RATE_PARAM].getValue();
		if (inputs[RATE_INPUT].isConnected())
			pitch += params[RATE_CV_PARAM].getValue() * inputs[RATE_INPUT].getVoltage();
		float rate = dsp::exp2_taylor5(clamp(pitch, -2.f, 16.f));

		// Expected event count this sample. Clamped so the sampler below stays bounded
		// even if rate exceeds the sample rate.
		float lambda = clamp(rate * args.sampleTime, 0.f, 8.f);

		// Knuth's Poisson sampler: cheap since lambda is typically << 1
		int k = 0;
		float threshold = std::exp(-lambda);
		float p = random::uniform();
		while (p > threshold) {
			k++;
			p *= random::uniform();
		}

		float amp = params[AMP_PARAM].getValue();
		bool bipolar = params[MODE_PARAM].getValue() > 0.5f;

		// Sum the impulses that landed in this sample; in bipolar mode each
		// event gets a random polarity.
		float out = 0.f;
		for (int i = 0; i < k; i++)
			out += (bipolar && random::uniform() < 0.5f) ? -amp : amp;
		outputs[NOISE_OUTPUT].setVoltage(out);

		if (k > 0)
			trigPulse.trigger(1e-3f);
		outputs[TRIG_OUTPUT].setVoltage(trigPulse.process(args.sampleTime) ? 10.f : 0.f);

		lights[EVENT_LIGHT].setBrightnessSmooth(k > 0, args.sampleTime);
	}
};

struct PoissonNoiseWidget : ModuleWidget {
	PoissonNoiseWidget(PoissonNoise* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/PoissonNoise.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 27.0)), module, PoissonNoise::RATE_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(9.0, 46.0)), module, PoissonNoise::RATE_CV_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21.5, 46.0)), module, PoissonNoise::RATE_INPUT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 66.0)), module, PoissonNoise::AMP_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(15.24, 85.0)), module, PoissonNoise::MODE_PARAM));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(15.24, 96.5)), module, PoissonNoise::EVENT_LIGHT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(9.0, 110.0)), module, PoissonNoise::TRIG_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.5, 110.0)), module, PoissonNoise::NOISE_OUTPUT));
	}
};

Model* modelPoissonNoise = createModel<PoissonNoise, PoissonNoiseWidget>("PoissonNoise");
