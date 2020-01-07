// This file is not meant to be compiled.
// This is a stub file to work nicely with the documentation generation system.

///# Documentation
// The Fog engine offers a way to automatically generate documentation for
// the engine, it can of course be used when making a game aswell. The syntax
// is very simple, inspired by markdown, and can be written without problems
// in a C++ file.


//// Different kinds of semantic comments
// There are 3 different kinds of comments.<br>
// The heading
 ///# title
// <br>
// The documentation
 ///* [optional name]
// The comment
 //// [optional name]
// <br>
//
// <p>
// Adding extra text on the line makes the text the title
// of that particular comment block. Where a comment block is
// a series of C line comments ( &#47;&#47; ) tightly packed. It
// might sound weird but it is quite intuitive, it's probably
// easiest if you learn from example.
// </p>
// <p>
// All these comments allow HTML in them, and the filtering is kept to a
// minimum. Slashes are filtered to in most places but that's nothing the good
// ol' <span style="text_decoration: underline;">& #47</span> can't fix.
// </p>
// <p>
// Also note that this isn't a heavy duty utility, it basically 
// just keeps running, no matter what you put in. And troubleshooting
// the parser will probably not be an ideal situation.
// </p>

//// The heading
// <p>
// A heading starts a section, one that can be collapsed in the side
// menu and given a lot of space. These are the "smallest buckets",
// where each folder is a sort of category, and the file generates
// the particular section, headings let you easily group things.
// </p>
// <p>
// I Like to think of "class" or "group of functionality" as having
// one heading. But it's often better to have to many than to few.
// </p>

//// The documentation
// A "documentation comment" can automatically generate an appropriate
// title and is made to be placed along side a function. These gobble
// up a block of comments and format them nicely along with the code block,
// preferably of the definition of the function. Examples are present in
// almost all header files in the project.

//// The Comment
// These are blocks of code that don't have to refer to a piece of code,
// these can be placed anywhere in the code base and will function just fine.

