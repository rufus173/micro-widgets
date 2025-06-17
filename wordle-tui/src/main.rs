use ratatui;
fn main() -> std::io::Result<()>{
	let mut term = ratatui::init();
	let res = run(&mut term);
	ratatui::restore();
	res
}
fn run(term: &mut ratatui::DefaultTerminal) ->std::io::Result<()>{
	loop {
		break Ok(());
	}
}
