#pragma once
#include "Engine/UI/Widget.hpp"
#include <vector>

class Panel : public Widget
{
public:
	Panel();
	virtual ~Panel();

	void Update(float deltaSeconds) override;
	void Render() const override;

protected:
	
};
