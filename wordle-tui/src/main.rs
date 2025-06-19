use ratatui;
use crossterm::event::{self, Event, KeyCode, KeyEvent, KeyEventKind};
use ratatui::layout::{Constraint,Layout};
use Constraint::{Fill, Length, Min};
use ratatui::widgets::Block;
fn main() -> std::io::Result<()>{
	let mut term = ratatui::init();
	let res = run(&mut term);
	ratatui::restore();
	res
}
fn run(term: &mut ratatui::DefaultTerminal) ->std::io::Result<()>{
	loop {
		term.draw(|frame| draw(frame));
		if handle_events()? {
			break Ok(());
		}
	}
}

fn handle_events() -> std::io::Result<bool> {
    match event::read()? {
        Event::Key(key) if key.kind == KeyEventKind::Press => match key.code {
            KeyCode::Char('q') => return Ok(true),
            // handle other key events
            _ => {}
        },
        // handle other events
        _ => {}
    }
    Ok(false)
}

fn draw(frame: &mut ratatui::Frame) {
	let keyboard = "\
┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐\n\
│ q │ w │ e │ r │ t │ y │ u │ i │ o │ p │\n\
└┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┘\n\
 │ a │ s │ d │ f │ g │ h │ j │ k │ l │\n\
 └─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴───┘\n\
   │ z │ x │ c │ v │ b │ n │ m │\n\
   └───┴───┴───┴───┴───┴───┴───┘\n\
"
    let vertical = Layout::vertical([Length(1), Min(0), Length(1)]);
    let [title_area, main_area, status_area] = vertical.areas(frame.area());
    let horizontal = Layout::horizontal([Fill(1); 2]);
    let [left_area, right_area] = horizontal.areas(main_area);

    frame.render_widget(Block::bordered().title("Title Bar"), title_area);
    frame.render_widget(Block::bordered().title("Status Bar"), status_area);
    frame.render_widget(Block::bordered().title("Left"), left_area);
    frame.render_widget(Block::bordered().title("Right"), right_area);
}
