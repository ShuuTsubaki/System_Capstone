function tadaEffect() {
	const tl = new TimelineMax();
	CustomWiggle.create("demoWiggle", {wiggles:8});
	tl.to(".tada", 0.15, {scale:0.90, rotation:-8});
	tl.to(".tada", 0.15, {scale:1.2, rotation:0, ease:Linear.easeNone}, "+=0.1");
	tl.to(".tada", 0.75, {rotation:3, ease:"demoWiggle"});
	tl.to(".tada", 0.15, {scale:1});
}