use glib::clone;
use gtk4::glib;
use gtk4::prelude::*;

pub struct Images {
	pixbufs: Vec<gtk4::gdk_pixbuf::Pixbuf>
}
impl Images {
	fn new(files: Vec<String>) -> Images{
		let images = Images {
			pixbufs: Vec::new()
		};
		for file in files{
			println!("{}",file);
		}
		images
	}
}

fn main() {
	let app = gtk4::Application::builder()
		.application_id("com.github.rufus173.dsply")
		.build();
	app.connect_activate(on_activate);
	app.run();
}

fn on_activate(application: &gtk4::Application){
	//====== load the images ======
	let images = Images::new(std::env::args().collect());
	//====== build the gui ======
	let window = gtk4::ApplicationWindow::builder()
		.application(application)
		.title("dsply")
		.resizable(false)
		.default_width(500)
		.default_height(500)
		.build();
	let close_button = gtk4::Button::with_label("Close");
	close_button.connect_clicked(
		clone!(#[strong] window, move |_|{
			window.close();
		})
	);
	window.set_child(Some(&close_button));
	window.present();
}
