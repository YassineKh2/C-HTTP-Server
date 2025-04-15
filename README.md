# HTTP Server

A simple HTTP Server built using C. This project demonstrates a basic web server implementation that handles GET, POST, PUT, and DELETE requests.

## Features

- **Static file serving:** Delivers HTML, CSS, images, etc.
- **Dynamic blog functionality:** Add, modify, and delete blog posts.
- **Basic routing:** Supports custom routes for different pages.
- **Polling and multiclient support:** Uses `poll()` to handle multiple connections concurrently.

## Project Structure

- **main.c:** Entry point of the server.
- **HelperFunctions.c:** Utility functions for string manipulation, file path resolution, and blog operations.
- **SeverFunctions.c:** Functions for setting up sockets, managing connections, and processing HTTP requests.
- **output/** 
  - **index.html:** Homepage.
  - **404.html:** Error page for undefined routes.
  - **style.css:** Basic styles for the site.
  - **blog/**
    - **blog.html:** Displays the list of blog posts.
    - **addblog.html:** Form to add a new blog post.
    - **modifyblog.html:** Form to modify an existing blog post.
    - **confirmation.html:** Confirmation after adding a blog.
    - **confirmationedit.html:** Confirmation after editing a blog.
    - **blogs.json:** JSON file storing blog post data.

## Building and Running

1. Compile the project:
    ````sh
    gcc main.c -o main
    ````
2. Run the server:
    ````sh
    ./main
    ````
3. In your browser, navigate to [http://localhost:3000](http://localhost:3000) to access your server.

### NOTE : Make sure you keep the same json structure !

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
```// filepath: /home/yassine/Desktop/Projects/HTTP Server/README.md
