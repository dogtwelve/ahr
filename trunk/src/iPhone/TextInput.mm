
#include <string.h>
#import "GLoftApp.h"
#import "TextInput.h"

extern "C" bool IsCharacterSupported(int c); 

static TextInput* g_sharedTextInputInstance = nil;

@implementation TextInput

- (id)initWithTextBuffer:(char*)textBuff WithSize:(unsigned int)buffSize AtX:(int)x Y:(int)y Width:(int)w Height:(int)h
{
	self = [super init];
	
	if (self != nil)
	{
		done = false;
		
		textBuffer = textBuff;
		textBufferSize = buffSize;
		
		CGRect rect;
		rect.origin.x = x;
		rect.origin.y = y;
		rect.size.width = w;
		rect.size.height = h;
		
		textField = [[UITextField alloc] initWithFrame:rect];
		
		textField.delegate = self;
		textField.borderStyle = UITextBorderStyleRoundedRect;
		textField.autocapitalizationType = UITextAutocapitalizationTypeAllCharacters;
		textField.keyboardType = UIKeyboardTypeASCIICapable;
		
		NSString* placeHolderString = [[NSString alloc] initWithUTF8String:textBuff];
		textField.placeholder = placeHolderString;
		[placeHolderString release];
		
		// When the user starts typing, show the clear button in the text field.
		textField.clearButtonMode = UITextFieldViewModeWhileEditing;
		
		textField.textColor = [UIColor blackColor];
		
		textField.clearsOnBeginEditing = YES;
		
		// Register to receive a notification when the movie has finished playing. 
		[[NSNotificationCenter defaultCenter] addObserver:self 
												 selector:@selector(textFieldDidChange:) 
													 name:UITextFieldTextDidChangeNotification 
												   object:textField];
		
		[[[GLoftApp sharedInstance] view] addSubview:textField];
		//[[[GLoftApp sharedInstance] view] bringSubviewToFront:textField];
	}
	
	return self;
}

+(TextInput*)getSharedInstance
{
	return g_sharedTextInputInstance;
}

+(void)setSharedInstance:(TextInput*)instance
{
	if (g_sharedTextInputInstance != nil && g_sharedTextInputInstance != instance)
		[g_sharedTextInputInstance release];
	
	g_sharedTextInputInstance = instance;
}

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
	return YES;
}

- (BOOL)isDone
{
	return done && !textField.editing;
}

- (BOOL)textFieldShouldReturn:(UITextField *)theTextField
{
	// When the user presses return, take focus away from the text field so that the keyboard is dismissed.
	if (theTextField == textField)
	{
		[textField resignFirstResponder];
		
        // Updating textBuffer
		const char* tmpStr = [textField.text UTF8String];
		strncpy(textBuffer, tmpStr, textBufferSize-1);
		textBuffer[textBufferSize-1] = 0;
		
		done = true;
	}
	
	return YES;
}

//  Notification called when the movie finished playing.
- (void) textFieldDidChange:(NSNotification*)notification
{	
	int length = [textField.text length];
	if (length <= 0)
		return;
	
	bool bTrimText = false;
	
	if (length >= textBufferSize)
	{
		const char* tmpStr = [textField.text UTF8String];
		strncpy(textBuffer, tmpStr, textBufferSize-1);
		textBuffer[textBufferSize-1] = 0;
		
		bTrimText = true;
	}
	else
	{		
		unsigned short lastChar = [textField.text characterAtIndex:(length - 1)];
	
		if (!IsCharacterSupported(lastChar))
		{
			//textField.text = [textField.text substringToIndex:(length - 1)];		
			if (length <= 1)
				textBuffer[0] = '\0';
			else
			{
				const char* tmpStr = [textField.text UTF8String];
				strncpy(textBuffer, tmpStr, length-1);
				textBuffer[length-1] = 0;
			}
			
			bTrimText = true;
		}
	}
	
	if (bTrimText)
	{
		NSString* placeHolderString = [[NSString alloc] initWithUTF8String:textBuffer];
		textField.text = placeHolderString;
		[placeHolderString release];		
	}
}

- (void)dealloc
{
	// remove movie notifications
	[[NSNotificationCenter defaultCenter] removeObserver:self
		name:UITextFieldTextDidChangeNotification
		object:textField];
	
	[textField removeFromSuperview];
	
	[textField release];
	
	[super dealloc];
}

@end


@implementation TextInputInitParams

@end


// Wrappers
extern "C" void TextInputStart(char* textBuf, unsigned int bufSize, int x, int y, int w, int h)
{
	TextInputInitParams* p = [[TextInputInitParams alloc] init];
	
	p->textBuff = (id)textBuf;
	p->buffSize = bufSize;
	p->x = x;
	p->y = y;
	p->w = w;
	p->h = h;
	
	[[GLoftApp sharedInstance] performSelectorOnMainThread:@selector(textInputInit:) withObject:p waitUntilDone:YES];
	
	[p release];
}

extern "C" void TextInputClose()
{
	[g_sharedTextInputInstance release];
	g_sharedTextInputInstance = nil;
}

extern "C" bool TextInputIsDone()
{
	return [g_sharedTextInputInstance isDone];
}

