#include "debouncedButton.h"

DebouncedButton * DebouncedButton::allButtons[MAX_DEBOUNCED_BUTTONS];
int DebouncedButton::debouncedButtonCount;

bool DebouncedButton::readButtonPress()
{
	return (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_RESET); // pressing the button means a zero
}

void DebouncedButton::buttonSysTick_Handler()
{
	bool buttonPressed = readButtonPress();
	static int buttonPressed_0;
	static int buttonPressed_1;
	const int debounceThreshold = 10;

	if (buttonPressed) {
		if (!buttonPressConfirmed)
		{
			buttonPressed_0 = 0;
			buttonPressed_1++;
			if (buttonPressed_1 > debounceThreshold) {
				buttonPressed_1 = debounceThreshold + 1;
				if (!buttonPressConfirmed) {
					buttonPressChanged = true;
				}
				buttonPressConfirmed = true;
			}
		}
	} else {
		if (buttonPressConfirmed)
		{
			buttonPressed_0++;
			buttonPressed_1 = 0;
			if (buttonPressed_0 > debounceThreshold) {
				buttonPressed_0 = debounceThreshold + 1;
				if (buttonPressConfirmed) {
					buttonPressChanged = true;
				}
				buttonPressConfirmed = false;
			}
		}
	}
}

bool DebouncedButton::buttonState(bool * changed)
{
	* changed = buttonPressChanged;
	buttonPressChanged = false; // only happens once
	return buttonPressConfirmed;
}

int DebouncedButton::addButton(DebouncedButton * button)
{
	int value=-1;
	if (debouncedButtonCount < MAX_DEBOUNCED_BUTTONS) 
	{
		value = DebouncedButton::debouncedButtonCount;
		DebouncedButton::allButtons[value] = button;
		DebouncedButton::debouncedButtonCount++;
	} else {
		Error_Handler();
	}
	return value;
}

void DebouncedButton::SysTick_Handler()
{
	for (int i = 0; i < DebouncedButton::debouncedButtonCount;i++)
	{
		DebouncedButton::allButtons[i]->buttonSysTick_Handler();
	}
}

// C interface implementation
extern "C" {
	void DebouncedButton_SysTick_Handler()
	{
		DebouncedButton::SysTick_Handler();
	}

	bool DebouncedButton_buttonState(int index, bool * changed)
	{
		DebouncedButton::allButtons[index]->buttonState(changed);
	}

	int DebouncedButton_addNewButton(GPIO_TypeDef  *GPIOx, uint16_t GPIO_Pin)
	{
		DebouncedButton * button = new DebouncedButton(GPIOx, GPIO_Pin);
		return DebouncedButton::addButton(button);
	}
}
