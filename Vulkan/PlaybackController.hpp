class PlaybackController {
public:
	// Constructor.
	PlaybackController();

	// Sets animation current time.
	void set_time_ratio(float _time);

	// Gets animation current time.
	float time_ratio() const;

	// Gets animation time ratio of last update. Useful when the range between
	// previous and current frame needs to pe processed.
	float previous_time_ratio() const;

	// Sets playback speed.
	void set_playback_speed(float _speed) { playback_speed_ = _speed; }

	// Gets playback speed.
	float playback_speed() const { return playback_speed_; }

	// Sets loop modes. If true, animation time is always clamped between 0 and 1.
	void set_loop(bool _loop) { loop_ = _loop; }

	// Gets loop mode.
	bool loop() const { return loop_; }

	// Updates animation time if in "play" state, according to playback speed and
	// given frame time _dt.
	// Returns true if animation has looped during update
	void Update(const ozz::animation::Animation& _animation, float _dt);

	// Resets all parameters to their default value.
	void Reset();

private:
	// Current animation time ratio, in the unit interval [0,1], where 0 is the
	// beginning of the animation, 1 is the end.
	float time_ratio_;

	// Time ratio of the previous update.
	float previous_time_ratio_;

	// Playback speed, can be negative in order to play the animation backward.
	float playback_speed_;

	// Animation play mode state: play/pause.
	bool play_;

	// Animation loop mode.
	bool loop_;
};

PlaybackController::PlaybackController()
	: time_ratio_(0.f),
	previous_time_ratio_(0.f),
	playback_speed_(1.f),
	play_(true),
	loop_(true) {}

void PlaybackController::Update(const ozz::animation::Animation& _animation,
	float _dt) {
	float new_time = time_ratio_;

	if (play_) {
		new_time = time_ratio_ + _dt * playback_speed_ / _animation.duration();
	}

	// Must be called even if time doesn't change, in order to update previous
	// frame time ratio. Uses set_time_ratio function in order to update
	// previous_time_ an wrap time value in the unit interval (depending on loop
	// mode).
	set_time_ratio(new_time);
}

void PlaybackController::set_time_ratio(float _time) {
	previous_time_ratio_ = time_ratio_;
	if (loop_) {
		// Wraps in the unit interval [0:1], even for negative values (the reason
		// for using floorf).
		time_ratio_ = _time - floorf(_time);
	}
	else {
		// Clamps in the unit interval [0:1].
		if (_time < 0.f) { _time = 0.f; }
		else if (_time > 1.f) { _time = 1.f; }

		//time_ratio_ = ozz::math::Clamp(0.f, _time, 1.f);
		time_ratio_ = _time;
	}
}

// Gets animation current time.
float PlaybackController::time_ratio() const { return time_ratio_; }

// Gets animation time of last update.
float PlaybackController::previous_time_ratio() const {
	return previous_time_ratio_;
}

void PlaybackController::Reset() {
	previous_time_ratio_ = time_ratio_ = 0.f;
	playback_speed_ = 1.f;
	play_ = true;
}